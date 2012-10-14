
#include "qdisplay/imout.hpp"

namespace jafar {
namespace qdisplay {

imout_t imout;

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
	
	if (image.width() > 0 && image.height() > 0)
	{
		image.setROI(0, 0, std::max(width,1), std::max(height,1));
		view->setImage(image);
		image.resetROI();
	}
}

}}

