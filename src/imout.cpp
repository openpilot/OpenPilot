#include "qdisplay/imout.hpp"

#include <highgui.h>

namespace jafar {
namespace image {

endl_ endl;
vsep_ vsep;
hsep_ hsep;
flush_ flush;
clear_ clear;

void oimstream::endl_fun()
{
	cursorx = 0;
	cursory = height;
}

void oimstream::enlargeImage(int width_factor, int height_factor)
{
	int newwidth = std::max(128, image.width()*width_factor);
	int newheight = std::max(128, image.height()*height_factor);
	image::Image newimage(newwidth, newheight, image.depth(), image.colorSpace());
	newimage.setTo(cv::Scalar(0));
	image.copy(newimage, 0, 0, 0, 0, width, height);
	image = newimage;
}

void oimstream::makeRoomFor(int pwidth, int pheight)
{
	const int factor = 2;
	pwidth += cursorx, pheight += cursory;
	int width_factor = (pwidth > image.width()) ? factor : 1;
	int height_factor = (pheight > image.height()) ? factor : 1;
	enlargeImage(width_factor, height_factor);
}

void oimstream::updateCursorsFor(int pwidth, int pheight)
{
	pwidth += cursorx, pheight += cursory;
	if (pwidth > width) width = pwidth;
	if (pheight > height) height = pheight;
	cursorx = pwidth;
}

oimstream& operator<<(oimstream& os, const image::Image &im)
{
	os.makeRoomFor(im.width(), im.height());
	im.copy(os.image, 0, 0, os.cursorx, os.cursory, im.width(), im.height());
	os.updateCursorsFor(im.width(), im.height());
	return os;
}	

oimstream& operator<<(oimstream& os, const endl_ &end)
{
	os.endl_fun();
	return os;
}

oimstream& operator<<(oimstream& os, const vsep_ &end)
{
	if (os.cursorx != 0) os.endl_fun();
	os.makeRoomFor(os.image.width(), 2);
	// TODO draw 2 lines instead of leaving 2 lines of background ?
	os.updateCursorsFor(os.image.width(), 2);
	os.endl_fun();
	return os;
}

oimstream& operator<<(oimstream& os, const hsep_ &end)
{
	os.makeRoomFor(2, os.height-os.cursory);
	// TODO draw 2 lines instead of leaving 2 cols of background ?
	os.updateCursorsFor(2, os.height-os.cursory);
	return os;
}

oimstream& operator<<(oimstream& os, const flush_ &end)
{
	os.flush_fun();
	return os;
}

oimstream& operator<<(oimstream& os, const clear_ &end)
{
	os.image.setTo(cv::Scalar(0));
	os.width = os.height = os.cursorx = os.cursory = 0;
	return os;
}


imout_t imout;

void imout_t::flush_fun()
{
	if (!created)
	{
		cv::namedWindow("image::imout", 1);// CV_WINDOW_NORMAL/* | CV_WINDOW_FREERATIO*/);
	}
	
	image.setROI(0, 0, width, height);
	cv::imshow("image::imout", image);
	image.resetROI();
}


}}


namespace jafar {
namespace qdisplay {

void imout_t::flush_fun()
{
	if (!viewer)
	{
		viewer = new Viewer();
		view = new ImageView();
		viewer->setImageView(view, 0, 0);
		viewer->resize(240, 320);
		//viewer->scene()->setSceneRect(0,0,size.width,size.height);
		viewer->setBackgroundColor(0,0,0);
		viewer->setTitle("qdisplay::imout");
	}
	
	image.setROI(0, 0, width, height);
	view->setImage(image);
	image.resetROI();
}

imout_t imout;

}}

