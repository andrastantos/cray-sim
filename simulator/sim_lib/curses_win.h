#ifndef __CURSES_WIN_H__
#define __CURSES_WIN_H__

#include "curses_wrapper.h"

namespace Curses {
	class Frame_c: public Panel_c {
	public:
		Frame_c(const char *aTitle, int aStartX, int aStartY, int aWidth, int aHeight):
			Panel_c(aStartX+1, aStartY+1, aWidth-1, aHeight-1),
			mBox(aStartX, aStartY, aWidth, aHeight)
		{
			Top(); // Fix the ordering of box and content
			SetTitle(aTitle);
		}
		Frame_c(const char *aTitle, const Window_c &aParent, int aOffsX, int aOffsY) :
			Panel_c(aParent.GetBeginX() + 1, aParent.GetBeginY() + 1, aParent.GetMaxX() - 2, aParent.GetMaxY() - 2),
			mBox(aParent.GetBeginX(), aParent.GetBeginY(), aParent.GetMaxX(), aParent.GetMaxY())
		{
			Top(); // Fix the ordering of box and content
			SetTitle(aTitle);
		}
		Frame_c(const Frame_c&) = delete;
		void SetTitle(const char *aTitle) {
			std::stringstream TitleStrm;
			TitleStrm << " " << aTitle << " ";
			mTitle = TitleStrm.str();
			RedrawBox();
		}
		virtual int GetMaxX() const { return mBox.GetMaxX(); }
		virtual int GetMaxY() const { return mBox.GetMaxY(); }
		virtual int GetBeginX() const { return mBox.GetBeginX(); }
		virtual int GetBeginY() const { return mBox.GetBeginY(); }
		virtual void Resize(int aWidth, int aHeight) {
			mBox.Resize(aWidth, aHeight);
			Panel_c::Resize(aWidth-2, aHeight-2);
			RedrawBox();
		}
		virtual void MoveWindow(int aStartX, int aStartY) {
			mBox.MoveWindow(aStartX, aStartY);
			Panel_c::MoveWindow(aStartX+1, aStartY+1);
		}
	protected:
		void RedrawBox() {
			mBox.Box(); // Draw the box around the main window
			if (mTitle.length() > 0) {
				int TitleStartX = std::max(int((mBox.GetMaxX() - mTitle.length()) / 2), int(0));
				mBox.AddStrN(mTitle.c_str(), mBox.GetMaxX(), TitleStartX, 0);
			}
		}
		Panel_c mBox;
		std::string mTitle;
	};

	// Frame can hold several sub-frames, and resize them intelligently to fit the overall frame
/****
	enum class PanelAlign_e {
		Top,
		TopRight,
		Right,
		BottomRight,
		Bottom,
		BottomLeft,
		Left,
		TopLeft,
		Center
	};
	class Tile_c : public Panel_c {
	public:
		Tile_c(
			class Frame_c &aParent, 
			int aMinWidth, 
			int aMaxWidth, 
			int aMinHeight, 
			int aMaxHeight,
			PanelAlign_e aAlignment
		) : mParent(&aParent), mMinWidth(aMinWidth), mMaxWidth(aMaxWidth), mMinHeight(aMinHeight), mMaxHeight(aMaxHeight), mAlignment(aAlignment) {}
	protected:
		Tile_c(
			int aMinWidth,
			int aMaxWidth,
			int aMinHeight,
			int aMaxHeight,
			PanelAlign_e aAlignment
		) : mParent(nullptr), mMinWidth(aMinWidth), mMaxWidth(aMaxWidth), mMinHeight(aMinHeight), mMaxHeight(aMaxHeight), mAlignment(aAlignment) {}

		class Frame_c *mParent;
		int mMinWidth;
		int mMaxWidth;
		int mMinHeight;
		int mMaxHeight;
		PanelAlign_e mAlignment;

		friend class Frame_c;

	};
	// Frames can be nested, so frames are tiles themselves
	class Frame_c : public Tile_c {
	public:
		Frame_c(Frame_c &aParent, int aMinWidth, int aMaxWidth, int aMinHeight, int aMaxHeight, PanelAlign_e aAlignment) : Tile_c(aParent, aMinWidth, aMaxWidth, aMinHeight, aMaxHeight, aAlignment), mOwnBorder(false) {}
		Frame_c(int aStartX, int aStartY, int aWidth, int aHeight) : Tile_c(aWidth, aWidth, aHeight, aHeight, PanelAlign_e::Center), mClientRegion(aWidth-2, aHeight-2), mOwnBorder(true) {}
		void AddTile(shared_ptr<Tile_c> aTile) {
			mTiles.push_back(aTile);
		}
		void Recalc();
	protected:
		vector<shared_ptr<Tile_c>> mTiles;
		Location_s mClientRegion;
		bool mOwnBorder;
	};
*****/
};

#endif // __CURSES_WIN_H__
