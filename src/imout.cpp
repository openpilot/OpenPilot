#include "qdisplay/imout.hpp"



namespace jafar {
namespace qdisplay {

endl_ endl;
vsep_ vsep;
hsep_ hsep;
flush_ flush;
clear_ clear;
oimstream imout;

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
	if (!os.viewer)
	{
		os.viewer = new Viewer();
		os.view = new ImageView();
		os.viewer->setImageView(os.view, 0, 0);
		os.viewer->resize(240, 320);
		//viewer->scene()->setSceneRect(0,0,size.width,size.height);
		os.viewer->setBackgroundColor(0,0,0);
		os.viewer->setTitle("imout");
	}
	
	os.image.setROI(0, 0, os.width, os.height);
	os.view->setImage(os.image);
	os.image.resetROI();
	return os;
}

oimstream& operator<<(oimstream& os, const clear_ &end)
{
	os.image.setTo(cv::Scalar(0));
	os.width = os.height = os.cursorx = os.cursory = 0;
	return os;
}


}}

