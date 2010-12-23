/* $Id:$ */
#ifndef _QDISPLAY_IOUT_HPP_
#define _QDISPLAY_IOUT_HPP_

#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageView.hpp"
#include "image/Image.hpp"

namespace jafar {
namespace image {

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
		image::Image image;
		int width, height;
		int cursorx, cursory;
	protected:
		void endl_fun();
		void enlargeImage(int width_factor, int height_factor);
		void makeRoomFor(int pwidth, int pheight);
		void updateCursorsFor(int pwidth, int pheight);
	protected:
		virtual void flush_fun() = 0;
	public:
		oimstream() {}
		virtual ~oimstream() {}
		
		friend oimstream& operator<<(oimstream& os, const image::Image &im);
		friend oimstream& operator<<(oimstream& os, const endl_ &end);
		friend oimstream& operator<<(oimstream& os, const vsep_ &end);
		friend oimstream& operator<<(oimstream& os, const hsep_ &end);
		friend oimstream& operator<<(oimstream& os, const flush_ &end);
		friend oimstream& operator<<(oimstream& os, const clear_ &end);
		
		void setBackgroundColor();// TODO
		void setSeparatorColor();// TODO
};

/*
 * FIXME window is never displayed if there is no cv::waitKey after flush...
 */
class imout_t: public oimstream
{
	protected:
		bool created;
	protected:
		virtual void flush_fun();
	public:
		imout_t(): created(false) {}
		~imout_t() {}
};

extern imout_t imout; ///< equivalent to std::cout for images

}}



namespace jafar {
namespace qdisplay {

class imout_t: public image::oimstream
{
	protected:
		qdisplay::Viewer *viewer;
		qdisplay::ImageView *view;
	protected:
		virtual void flush_fun();
	public:
		imout_t(): viewer(NULL), view(NULL) {}
		~imout_t() { delete viewer; delete view; }
};

extern imout_t imout; ///< equivalent to std::cout for images

}}


#endif

