


// jafar debug include
#include "kernel/jafarDebug.hpp"
#include "kernel/jafarTestMacro.hpp"

#include "qdisplay/init.hpp"
#include "qdisplay/Viewer.hpp"
#include "qdisplay/ImageView.hpp"
#include "qdisplay/Shape.hpp"
#include "kernel/threads.hpp"

#include <iostream>

using namespace jafar;
using namespace std;

#define PI 3.141592

#define TRACE std::cout << __FILE__ << ":" << __LINE__ << std::endl;

/******************************************************************************/

void display_mono_unique(void*)
{
	TRACE; qdisplay::Viewer *viewer = new qdisplay::Viewer;
	TRACE; image::Image img;
	TRACE; img.load("demo_suite/qt.png");
	TRACE; qdisplay::ImageView *view = new qdisplay::ImageView(img);
	TRACE; view->setImage(img);
	TRACE; viewer->setImageView(view, 0, 0);
	
	int t = 0;
	qdisplay::Shape *s = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, 20*(1.5+cos(2*PI*t/15)), 20*(1.5+sin(2*PI*t/15)), 3, 3);
	s->setVisible(true);
	s->setColor(255, 0, 0); // red
	view->addShape(s);
	while (1)
	{
		std::cout << t << std::endl;
		++t;
		s->setPos(20*(1.5+cos(2*PI*t/15)), 20*(1.5+sin(2*PI*t/15)));
		
//		sleep(1); QApplication::instance()->processEvents();
		qdisplay::qtSleep(1000);
	}
}

/******************************************************************************/

void display_mono_periodic(void*)
{
	static int t = 0;
	static qdisplay::Shape *s = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, 20*(1.5+cos(2*PI*t/15)), 20*(1.5+sin(2*PI*t/15)), 3, 3);
	if (t == 0)
	{
		TRACE; qdisplay::Viewer *viewer = new qdisplay::Viewer;
		TRACE; image::Image img;
		TRACE; img.load("demo_suite/qt.png");
		TRACE; qdisplay::ImageView *view = new qdisplay::ImageView(img);
		TRACE; view->setImage(img);
		TRACE; viewer->setImageView(view, 0, 0);
		
		s->setVisible(true);
		s->setColor(255, 0, 0); // red
		view->addShape(s);
	}
	else
	{
		std::cout << t << std::endl;
		s->setPos(20*(1.5+cos(2*PI*t/15)), 20*(1.5+sin(2*PI*t/15)));
	}
	++t;
}

/******************************************************************************/

struct Stereo_struct
{
	int t;
	qdisplay::Shape *s;
	kernel::FifoMutex mutex;
	Stereo_struct(): t(0),s(NULL) {}
} stereo_struct;

void main_stereo(Stereo_struct *sparam)
{
	for(int i = 0; i < 100; i++)
	{
		TRACE; sparam->mutex.lock();
		TRACE; sparam->t++;
		TRACE; sleep(1);
		TRACE; sparam->mutex.unlock();
	}
}

/******************************************************************************/

void display_stereo_unique(Stereo_struct *sparam)
{
	while(true)
	{
		TRACE; sparam->mutex.lock();
		if (sparam->s == NULL)
		{
			TRACE; sparam->s = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, 20*(1.5+cos(2*PI*sparam->t/15)), 20*(1.5+sin(2*PI*sparam->t/15)), 3, 3);
			TRACE; qdisplay::Viewer *viewer = new qdisplay::Viewer;
			TRACE; image::Image img;
			TRACE; img.load("demo_suite/qt.png");
			TRACE; qdisplay::ImageView *view = new qdisplay::ImageView(img);
			TRACE; view->setImage(img);
			TRACE; viewer->setImageView(view, 0, 0);
			
			TRACE; sparam->s->setVisible(true);
			TRACE; sparam->s->setColor(255, 0, 0); // red
			TRACE; view->addShape(sparam->s);
		}
		else
		{
			TRACE; std::cout << sparam->t << std::endl;
			TRACE; sparam->s->setPos(20*(1.5+cos(2*PI*sparam->t/15)), 20*(1.5+sin(2*PI*sparam->t/15)));
		}
		TRACE; sparam->mutex.unlock();
		
		qdisplay::qtSleep(1000);
//		sleep(1); QApplication::instance()->processEvents();
	}
	
}

/******************************************************************************/


void display_stereo_periodic(Stereo_struct *sparam)
{
	TRACE; sparam->mutex.lock();
	if (sparam->s == NULL)
	{
		TRACE; sparam->s = new qdisplay::Shape(qdisplay::Shape::ShapeRectangle, 20*(1.5+cos(2*PI*sparam->t/15)), 20*(1.5+sin(2*PI*sparam->t/15)), 3, 3);
		TRACE; qdisplay::Viewer *viewer = new qdisplay::Viewer;
		TRACE; image::Image img;
		TRACE; img.load("demo_suite/qt.png");
		TRACE; qdisplay::ImageView *view = new qdisplay::ImageView(img);
		TRACE; view->setImage(img);
		TRACE; viewer->setImageView(view, 0, 0);
		
		TRACE; sparam->s->setVisible(true);
		TRACE; sparam->s->setColor(255, 0, 0); // red
		TRACE; view->addShape(sparam->s);
	}
	else
	{
		TRACE; std::cout << sparam->t << std::endl;
		TRACE; sparam->s->setPos(20*(1.5+cos(2*PI*sparam->t/15)), 20*(1.5+sin(2*PI*sparam->t/15)));
	}
	TRACE; sparam->mutex.unlock();
}


/******************************************************************************/

int main()
{
//	qdisplay::QtAppStart qas(&display_mono_unique,NULL,0, NULL);
//	qdisplay::QtAppStart qas(&display_mono_periodic,NULL,1000, NULL);
//	qdisplay::QtAppStart((qdisplay::FUNC)&display_stereo_unique,(qdisplay::FUNC)&main_stereo,0, &stereo_struct);
	qdisplay::QtAppStart((qdisplay::FUNC)&display_stereo_periodic,0, (qdisplay::FUNC)&main_stereo, 0, 1000,&stereo_struct);

}

