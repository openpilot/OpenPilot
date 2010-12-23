/* $Id:$ */
#ifndef _QDISPLAY_IOUT_HPP_
#define _QDISPLAY_IOUT_HPP_

#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageView.hpp"
#include "image/Image.hpp"

namespace jafar {
namespace qdisplay {

struct endl_ {};
struct vsep_ {};
struct hsep_ {};
struct flush_ {};
struct clear_ {};

extern endl_ endl; ///< end of line
extern vsep_ vsep; ///<  vertical separator (add a 2 pixels horizontal line)
extern hsep_ hsep; ///<  horizontal separator  (add a 2 pixels vertical line)
extern flush_ flush; ///< send to the screen
extern clear_ clear; ///< clear the display

class oimstream
{
	protected:
		qdisplay::Viewer *viewer;
		qdisplay::ImageView *view;
		image::Image image;
		int width, height;
		int cursorx, cursory;
	protected:
		void endl_fun();
		void enlargeImage(int width_factor, int height_factor);
		void makeRoomFor(int pwidth, int pheight);
		void updateCursorsFor(int pwidth, int pheight);
	public:
		oimstream(): viewer(NULL), view(NULL) {}
		~oimstream() { delete viewer; delete view; }
		friend oimstream& operator<<(oimstream& os, const image::Image &im);
		friend oimstream& operator<<(oimstream& os, const endl_ &end);
		friend oimstream& operator<<(oimstream& os, const vsep_ &end);
		friend oimstream& operator<<(oimstream& os, const hsep_ &end);
		friend oimstream& operator<<(oimstream& os, const flush_ &end);
		friend oimstream& operator<<(oimstream& os, const clear_ &end);
		
		void setBackgroundColor();// TODO
		void setSeparatorColor();// TODO
};

extern oimstream imout; ///< equivalent to std::cout for images


}}


#endif

