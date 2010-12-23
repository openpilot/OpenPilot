/* $Id:$ */
#ifndef _QDISPLAY_IMOUT_HPP_
#define _QDISPLAY_IMOUT_HPP_

#include "image/oimstream.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageView.hpp"


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

