#ifndef __CURSES_WRAPPER_H__
#define __CURSES_WRAPPER_H__

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif //NOMINMAX
#include <windows.h>
#undef MOUSE_MOVED
#else
#include <time.h>
#endif
#include <curses.h>
#include <panel.h>
#include <string>
#include <vector>
#include <exception>
#include "utils.h"
#include <vector>
#include <iostream>

namespace Curses {
	struct Location_s {
		Location_s(): X(0), Y(0) {}
		explicit Location_s(int aX, int aY): X(aX), Y(aY) {}
		int X;
		int Y;
	};

	class Window_c : public std::ostream {
	protected:
		void Setup() {
			NoDelay(true);
			Keypad(true);
		}
	public:
		explicit Window_c(WINDOW *aWnd) : std::ostream(&mStreamBuf), mWnd(aWnd), mOwner(false), mStreamBuf(*this), mIsScrollOk(false) {
			Setup();
		}
		Window_c(WINDOW *aWnd, bool aOwner) : std::ostream(&mStreamBuf), mWnd(aWnd), mOwner(aOwner), mStreamBuf(*this), mIsScrollOk(false) {
			Setup();
		}
		Window_c(const Window_c&) = delete;
		Window_c(int aStartX, int aStartY, int aWidth, int aHeight) : std::ostream(&mStreamBuf), mStreamBuf(*this), mIsScrollOk(false) {
			mWnd = newwin(aHeight, aWidth, aStartY, aStartX);
			if (mWnd == nullptr) throw Generic_x("newwin failed");
			mOwner = true;
			Setup();
		}
		Window_c(const Window_c &aParent, int aOffsX, int aOffsY) : std::ostream(&mStreamBuf), mStreamBuf(*this), mIsScrollOk(false){
			mWnd = newwin(aParent.GetMaxY() - 2 * aOffsY, aParent.GetMaxX() - 2 * aOffsX, aParent.GetBeginY() + aOffsY, aParent.GetBeginX() + aOffsX);
			if (mWnd == nullptr) throw Generic_x("subwin failed");
			mOwner = true;
			Setup();
		}
		virtual ~Window_c() {
			if (mOwner) delwin(mWnd);
		}
		void NoDelay(bool aNoDelay) {
			nodelay(mWnd, aNoDelay);
		}
		void Keypad(bool aKeypad) {
			keypad(mWnd, aKeypad);
		}
		virtual int GetMaxX() const { return getmaxx(mWnd); }
		virtual int GetMaxY() const { return getmaxy(mWnd); }
		virtual int GetBeginX() const { return getbegx(mWnd); }
		virtual int GetBeginY() const { return getbegy(mWnd); }
		Location_s GetMax() { return Location_s(GetMaxX(), GetMaxY()); }
		void Move(int aX, int aY) { 
			if (wmove(mWnd,aY,aX) != OK) throw Generic_x("Move failed"); 
		}
		void Move(const Location_s &aLoc) { Move(aLoc.X, aLoc.Y); }
		virtual void MoveWindow(int aStartX, int aStartY) {
			if (mvwin(mWnd, aStartY, aStartX) != OK) throw Generic_x("mvwin failed");
		}
		void MoveWindow(const Location_s &aLoc) { MoveWindow(aLoc.X, aLoc.Y); }
		virtual void Resize(int aWidth, int aHeight) {
			if (wresize(mWnd, aHeight, aWidth) != OK) throw Generic_x("Resize failed");
			Clear();
			Move(0, 0);
		}
		void Resize(const Location_s &aLoc) { Resize(aLoc.X, aLoc.Y); }
		void Grow(int aWidth, int aHeight) { Resize(GetMaxX() + aWidth, GetMaxY() + aHeight); }
		void Grow(const Location_s &aLoc) { Resize(GetMaxX() + aLoc.X, GetMaxY() + aLoc.Y); }
		void AddCh(chtype aCh) {
			if (waddch(mWnd, aCh) != OK) throw Generic_x("AddCh failed");
		}
		chtype InCh() {
			return winch(mWnd);
		}
		bool AddChNoThrow(chtype aCh) throw() {
			return (waddch(mWnd, aCh) == OK);
		}
		void AddStr(const char *aCh) {
			if (waddstr(mWnd, aCh) != OK) throw Generic_x("AddStr failed");
		}
		void AddStr(const std::string &aCh) { AddStr(aCh.c_str()); }
		void AddStrN(const char *aCh, size_t aLen) {
			if (waddnstr(mWnd, aCh, int(aLen)) != OK) throw Generic_x("AddStrN failed");
		}
		void MvAddStr(const char *aCh, int aX, int aY) {
			if (mvwaddstr(mWnd, aY, aX, aCh) != OK) throw Generic_x("MvAddStr failed");
		}
		void MvAddStr(const std::string &aCh, int aX, int aY) { MvAddStr(aCh.c_str(), aX, aY); }
		void MvAddStr(const char *aCh, const Location_s &aLoc) { MvAddStr(aCh, aLoc.X, aLoc.Y); }
		void MvAddStr(const std::string &aCh, const Location_s &aLoc) { MvAddStr(aCh.c_str(), aLoc.X, aLoc.Y); }
		void MvAddStrN(const char *aCh, size_t aLen, int aX, int aY) {
			if (mvwaddnstr(mWnd, aY, aX, aCh, int(aLen)) != OK) throw Generic_x("MvAddStrN failed");
		}
		void MvAddStrN(const char *aCh, size_t aLen, const Location_s &aLoc) { MvAddStrN(aCh, aLen, aLoc.X, aLoc.Y); }
		void AddStr(const char *aCh, int aX, int aY) {
			Location_s CurLoc = GetCur();
			MvAddStr(aCh, aX, aY);
			Move(CurLoc);
		}
		void AddStr(const std::string &aCh, int aX, int aY) { AddStr(aCh.c_str(), aX, aY); }
		void AddStr(const char *aCh, const Location_s &aLoc) { AddStr(aCh, aLoc.X, aLoc.Y); }
		void AddStr(const std::string &aCh, const Location_s &aLoc) { AddStr(aCh.c_str(), aLoc.X, aLoc.Y); }
		void AddStrN(const char *aCh, size_t aLen, int aX, int aY) {
			Location_s CurLoc = GetCur();
			MvAddStrN(aCh, aLen, aX, aY);
			Move(CurLoc);
		}
		void AddStrN(const char *aCh, size_t aLen, const Location_s &aLoc) { AddStrN(aCh, aLen, aLoc.X, aLoc.Y); }
		void ScrollOk(bool aScrollOk) {
			if (scrollok(mWnd, aScrollOk) != OK) throw Generic_x("ScrollOk failed");
			mIsScrollOk = aScrollOk;
		}
		bool IsScrollOk() const { return mIsScrollOk; }
		int GetCurX() const { return getcurx(mWnd); }
		int GetCurY() const { return getcury(mWnd); }
		Location_s GetCur() const { return Location_s(GetCurX(),GetCurY()); }
		void Clear() {
			if (wclear(mWnd) != OK) throw Generic_x("Clear failed"); 
			clear();
		}
		void ClearToBot() {
			if (wclrtobot(mWnd) != OK) throw Generic_x("ClearToBot failed");
		}
		void ClearToEol() {
			if (wclrtoeol(mWnd) != OK) throw Generic_x("ClearToEol failed");
		}
		virtual void Refresh() {
			if (wrefresh(mWnd) != OK) throw Generic_x("Refresh failed"); 
		}
		void PushCur(int aX, int aY) {
			PushCur(Location_s(aX, aY));
		}
		void PushCur(const Location_s &aLoc) {
			mLocationStack.push_back(GetCur());
			Move(aLoc);
		}
		void PopCur() {
			if (mLocationStack.empty()) return;
			Move(mLocationStack.back());
			mLocationStack.pop_back();
		}
		bool IsKeyPressed(int &aCh) {
			aCh = wgetch(mWnd);
			if (aCh == ERR) {
				return false;
			}
			return true;
		}
		void Scroll() {
			if (scroll(mWnd) != OK)
				throw Generic_x("Scroll failed");
		}
		void CopyRegion(const Location_s &aSrc, const Location_s &aDst, const Location_s &aSize, bool aOverlay) {
			if (aSize.Y < 1) return;
			Location_s LastCurPos = GetCur();
			std::vector<chtype> Line((aSize.X+1)*(aSize.Y+1));
			for (int Y=0;Y<aSize.Y;++Y) {
				for(int X=0;X<aSize.X;++X) {
					Move(Location_s(aSrc.X+X,aSrc.Y+Y));
					Line[Y*aSize.X+X] = winch(mWnd);
				}
			}
			for (int Y=0;Y<aSize.Y;++Y) {
				Move(Location_s(aDst.X,aDst.Y+Y));
				for(int X=0;X<aSize.X;++X) {
					if (waddch(mWnd, Line[Y*aSize.X+X]) != OK) throw Generic_x("waddch failed");
				}
			}
			Move(LastCurPos);
/*			if (
				copywin(mWnd, mWnd,
					aSrc.Y, aSrc.X,
					aDst.Y, aDst.X,
					aDst.Y+aSize.Y, aDst.X+aSize.X,
					aOverlay
				) != OK
			) {
				stringstream Str;
				Str << "CopyRegion failed with parameters:" << endl <<
					" aSrc.Y=" << aSrc.Y << " aSrc.X=" << aSrc.X << endl <<
					" aDst.Y=" << aDst.Y << " aDst.X=" << aDst.X << endl <<
					" aSize.Y=" << aSize.Y << " aSize.X=" << aSize.X << endl <<
					" aDst.Y+aSize.Y=" << aDst.Y+aSize.Y << " aDst.X+aSize.X=" << aDst.X+aSize.X << endl <<
					" begy=" << getbegy(mWnd) << " begx=" << getbegx(mWnd) << endl <<
					" maxy=" << getmaxy(mWnd) << " maxx=" << getmaxx(mWnd) << endl <<
					" aOverlay=" << aOverlay;
				throw Generic_x(Str.str());
			}*/
		}
		virtual int GetCh() { return wgetch(mWnd); }
		void Timeout(int aTimeout) { wtimeout(mWnd, aTimeout); }
		void Box(chtype aVerticalCh = ACS_VLINE, chtype aHorizontalCh = ACS_HLINE) {
			if (box(mWnd, aVerticalCh, aHorizontalCh)) throw Generic_x("Box failed");
		}
		void AttrSet(chtype aAttr) {
			if (wattrset(mWnd, aAttr) != OK) throw Generic_x("AttrSet failed");
		}
		void AttrOn(chtype aAttr) {
			if (wattron(mWnd, aAttr) != OK) throw Generic_x("AttrOn failed");
		}
		void AttrOff(chtype aAttr) {
			if (wattroff(mWnd, aAttr) != OK) throw Generic_x("AttrOff failed");
		}
		void BkgdSet(chtype aAttr) {
			wbkgdset(mWnd, aAttr);
		}
/*		Window_c &operator << (basic_ostream<char, char_traits<char> >& (*aFormatterFn)(basic_ostream<char, char_traits<char> >&)) {
			stringstream StrStrm;
			aFormatterFn(StrStrm);
			AddStr(StrStrm.str());
			if (StrStrm.str() == "\n") {
				Refresh();
			}
			return *this;
		}
		template <typename tElement> Window_c &operator << (const tElement &aElement) {
			stringstream StrStrm;
			StrStrm << aElement;
			AddStr(StrStrm.str());
			return *this;
		};*/

		friend class SubWindow_c;
		friend class Panel_c;
	protected:
		// A small streambuf derivative to support ostream-type interfaces
		class StreamBuf_c : public std::streambuf
		{
		public:
			explicit StreamBuf_c(Window_c &aParent) : mParent(aParent) {}
			virtual int overflow(int aChar = EOF) override {
				if (!mParent.AddChNoThrow(aChar)) return EOF;
				return aChar;
			}
			virtual std::streamsize xsputn(const char *aStr, std::streamsize aCount) override {
				try {
					mParent.AddStrN(aStr, static_cast<size_t>(aCount));
				}
				catch (Generic_x &) {
					return 0;
				}
				return aCount;
			}
			virtual int sync() override {
				try {
					mParent.Refresh();
				}
				catch (Generic_x &) {
					return -1;
				}
				return 0;
			}
		protected:
			Window_c &mParent;
		};

		WINDOW *mWnd;
		bool mOwner;
		StreamBuf_c mStreamBuf;
		bool mIsScrollOk;

		std::vector<Location_s> mLocationStack;
	};

	class Session_c: public Window_c {
	public:
		explicit Session_c(bool aDoInit = true): Window_c(initscr(), false), mInitialized(false) {
			if (aDoInit) Open();
		}
		~Session_c() {
			Close();
		}
		void Open() {
			if (!mInitialized) {
				cbreak();
				noecho();
				nodelay(mWnd,true);
				scrollok(mWnd,true);
				keypad(mWnd, true);
				mOriginalSize = Location_s(getmaxx(mWnd), getmaxy(mWnd));
			}
			mInitialized = true;
		}
		void Close() {
			if (mInitialized) {
				ResizeTerm(mOriginalSize);
//				if (isendwin()) {
					clear();
					refresh();
					endwin();
//				}
			}
			mInitialized = false;
		}
		Session_c(const Session_c&) = delete;
		void ResizeTerm() { ResizeTerm(0, 0); }
		void ResizeTerm(size_t aX, size_t aY) {
			if (resize_term(int(aY), int(aX)) != OK) throw Generic_x("Terminal resize failed");
		}
		void ResizeTerm(const Location_s &aLoc) { ResizeTerm(aLoc.X, aLoc.Y); }
		void Echo() { echo(); }
		void NoEcho() { noecho(); }
		void NoDelay(bool aDelay) { nodelay(mWnd,aDelay); }
	protected:
		Location_s mOriginalSize;
		bool mInitialized;
	};

	class SubWindow_c : public Window_c {
	public:
		SubWindow_c(Window_c &aParent, int aOffsX, int aOffsY) : Window_c(nullptr, true), mParent(aParent) {
			mWnd = subwin(mParent.mWnd, mParent.GetMaxY() - 2*aOffsY, mParent.GetMaxX() - 2*aOffsX, mParent.GetBeginY() + aOffsY, mParent.GetBeginX() + aOffsX);
			if (mWnd == nullptr) throw Generic_x("subwin failed");
		}
		SubWindow_c(Window_c &aParent, int aBeginX, int aBeginY, int aWidth, int aHeight) : Window_c(nullptr, true), mParent(aParent) {
			mWnd = subwin(mParent.mWnd, aHeight, aWidth, aBeginY, aBeginX);
			if (mWnd == nullptr) throw Generic_x("subwin failed");
		}
		SubWindow_c(const SubWindow_c&) = delete;
		virtual void Refresh() override {
			touchwin(mParent.mWnd);
			mParent.Refresh();
		}
	protected:
		Window_c &mParent;
	};

	class Panel_c: public Window_c {
	public:
		Panel_c(Session_c &aSession) : Window_c(aSession.mWnd, false) {
			mPanel = new_panel(mWnd);
			if (mPanel == nullptr) throw Generic_x("new_panel failed");
			set_panel_userptr(mPanel, this);
		}
		Panel_c(int aStartX, int aStartY, int aWidth, int aHeight): Window_c(aStartX, aStartY, aWidth, aHeight) {
			mPanel = new_panel(mWnd);
			if (mPanel == nullptr) throw Generic_x("new_panel failed");
			set_panel_userptr(mPanel, this);
		}
		Panel_c(const Window_c &aParent, int aOffsX, int aOffsY): Window_c(aParent, aOffsX, aOffsY) {
			mPanel = new_panel(mWnd);
			if (mPanel == nullptr) throw Generic_x("new_panel failed");
			set_panel_userptr(mPanel, this);
		}
		Panel_c(const Panel_c&) = delete;
		virtual ~Panel_c() {
			del_panel(mPanel);
		}
		void Bottom() {
			if (bottom_panel(mPanel) != OK) throw Generic_x("Bottom failed");
		}
		void Top() {
			if (top_panel(mPanel) != OK) throw Generic_x("Bottom failed");
		}
		void Hide() {
			if (hide_panel(mPanel) != OK) throw Generic_x("Hide failed");
		}
		void Show() {
			if (show_panel(mPanel) != OK) throw Generic_x("Show failed");
		}
		virtual void MoveWindow(int aStartX, int aStartY) {
			if (move_panel(mPanel, aStartY, aStartX) != OK) throw Generic_x("MoveWindow failed on panel");
		}
		const Panel_c *Above() const {
			PANEL *PanelAbove = panel_above(mPanel);
			if (PanelAbove == nullptr) throw Generic_x("Above failed");
			const Panel_c *RetVal = reinterpret_cast<const Panel_c*>(panel_userptr(PanelAbove));
			if (RetVal == nullptr) throw Generic_x("Above is not a Panel_c");
			return RetVal;
		}
		const Panel_c *Below() const {
			PANEL *PanelBelow = panel_below(mPanel);
			if (PanelBelow == nullptr) throw Generic_x("Above failed");
			const Panel_c *RetVal = reinterpret_cast<const Panel_c*>(panel_userptr(PanelBelow));
			if (RetVal == nullptr) throw Generic_x("Above is not a Panel_c");
			return RetVal;
		}
		bool IsHidden() { return panel_hidden(mPanel) == OK; }
		static void Update() {
			update_panels();
			doupdate();
		}
		virtual void Refresh() {
			Update();
		}
	protected:
		PANEL *mPanel;
	};
};

#endif // __CURSES_WRAPPER_H__
