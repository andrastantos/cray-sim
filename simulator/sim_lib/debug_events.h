#ifndef __DEBUG_EVENTS_H__
#define __DEBUG_EVENTS_H__

#include <stdlib.h>
#include <math.h>
#include "utils.h"
#include "cray_types.h"
#include <stdint.h>
#include <string.h>
#include <vector>
#include <map>
#include <limits.h>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <regex>
#include "cray_logger.h"
#include "config_file.h"

typedef std::string DebugEvent_t;

class DebugEventDispatcher_c {
public:
	DebugEventDispatcher_c() {}
	virtual ~DebugEventDispatcher_c() {}

	class DebugEventHandler_i {
	public:
		virtual ~DebugEventHandler_i() {}
		virtual void Handle(const DebugEvent_t &aEvent) = 0;
		virtual bool operator == (const void *aRealHandler) const = 0;
	};

	template <typename tParentClass> class DebugEventHandler_t: public DebugEventHandler_i {
	public:
		explicit DebugEventHandler_t() {}
		DebugEventHandler_t(tParentClass &aParentClass): mParentClass(aParentClass) {}
		virtual void Handle(const DebugEvent_t &aEvent) override { mParentClass.HandleDebugEvent(aEvent); }
		virtual bool operator == (const void *aRealHandler) const override { return &mParentClass == aRealHandler; }
	protected:
		tParentClass &mParentClass;
	};

	virtual void Fire(const DebugEvent_t &aEvent) const {
		for (auto EventHandler : mHandlers ) {
			EventHandler->Handle(aEvent);
		}
	}


	template <typename tHandlerClass> void AddHandler(tHandlerClass &aHandler) { std::shared_ptr<DebugEventHandler_i> HandlerShim = std::make_shared<DebugEventHandler_t<tHandlerClass> >(aHandler); mHandlers.push_back(HandlerShim); }
protected:
	std::vector<std::shared_ptr<DebugEventHandler_i>> mHandlers;
};

template <typename tBreakPoint> class EventPoints_t {
public:
	EventPoints_t() {}
	virtual ~EventPoints_t() {}
	void LoadEventPoints(const Configuration_c &aConfig) {
		try {
			for(const auto &EventPoint: aConfig) {
				std::string BreakPointType = EventPoint.second.get<std::string>("Type");
				DebugEvent_t Event = EventPoint.first;
				mBreakPoints.push_back(BreakPointEntry_s(Event,CreateBreakPoint(BreakPointType,EventPoint.second)));
			}
		}
		catch(std::exception &) {
			mBreakPoints.clear();
			throw;
		}
	};
	void Fire(const DebugEvent_t &aEvent) {
		for(auto Entry: mBreakPoints) {
			if (std::regex_search(aEvent,Entry.Event)) FireBreakPoint(Entry.BreakPoint.get());
		}
	}
protected:
	virtual void FireBreakPoint(tBreakPoint *aBreakPoint) = 0;
	virtual std::shared_ptr<tBreakPoint> CreateBreakPoint(const std::string &aBreakPointType, const Configuration_c &aConfig) = 0;

	struct BreakPointEntry_s {
		std::regex Event;
		std::shared_ptr<tBreakPoint> BreakPoint;

		BreakPointEntry_s(const DebugEvent_t &aEvent, const std::shared_ptr<tBreakPoint> &aBreakPoint): Event(aEvent), BreakPoint(aBreakPoint) {}
	};

	std::vector<BreakPointEntry_s> mBreakPoints;
};

#endif // __DEBUG_EVENTS_H__