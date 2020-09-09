#ifndef __UI_H__
#define __UI_H__

#include <stdint.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/function.hpp>
#include "cray_types.h"
#include "exceptions.h"
#include "utils.h"
#include "commands.h"

// Abstract interface to implement I/O channels
enum class PeripheralType_e {
	Disk,
	Tape,
	Network,
	Console,
	Expander,
	Bmx,
	Printer,
	Other
};

extern const char *PeripheralTypeNames[];

typedef boost::property_tree::ptree StatusReport_c;

/*
struct UIHook_s {
	boost::function<void()> Callback;
	int TriggerKey;
};
*/

typedef std::vector<std::unique_ptr<CmdFactoryBase_i>> CommandHooks_t;

#endif // __UI_H__
