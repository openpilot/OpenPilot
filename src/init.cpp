#include "qdisplay/init.hpp"

#include <QApplication>
#include <QTimer>
#include <QTime>
 
//#include <boost/thread/thread.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "qdisplay/Viewer.hpp"

//#include <typeinfo>

namespace jafar {
namespace qdisplay {

	QtAppStart::QtAppStart(void (*_display)(SharedDataStructure*), int prioDisplay,
		void (*_main)(SharedDataStructure*), int prioMain,
		int display_interval, SharedDataStructure *_sharedDataStructure, void (*_onExit)(SharedDataStructure*,boost::thread*)):
		main_(_main), display_(_display), onExit_(_onExit), thread_main(NULL), app(NULL), timer(NULL), 
		sharedDataStructure(_sharedDataStructure)
	{
		if (main_) 
		{
			kernel::setCurrentThreadPriority(prioMain);
			thread_main = new boost::thread(boost::bind(main_,sharedDataStructure));
		}
		kernel::setCurrentThreadPriority(prioDisplay);
		
		int argc = 0;
		app = new QApplication(argc, NULL);
		//app->setQuitOnLastWindowClosed(false);
		if (display_interval == 0)
		{
			QTimer::singleShot(0, this, SLOT(display()));
		} else
		{
			timer = new QTimer;
			timer->connect(timer, SIGNAL(timeout()), this, SLOT(display()));
			timer->start(display_interval);
		}
		app->connect(app, SIGNAL(lastWindowClosed()), this, SLOT(onExit()));
		//std::cout << "app->exec()" << std::endl;
		app->exec();
	}

	QtAppStart::~QtAppStart()
	{
		delete timer;
		delete app;
		delete thread_main;
	}

	void QtAppStart::display()
	{
		//std::cout << "timer fire" << std::endl;
// 		if (thread_main && thread_main->timed_join(boost::posix_time::milliseconds (0))) app->exit();
		(*display_)(sharedDataStructure);
	}
	
	void QtAppStart::onExit()
	{
		if (onExit_) (*onExit_)(sharedDataStructure, thread_main);
	}

void qtSleep(int ms)
{
	QTime dieTime = QTime::currentTime().addMSecs(ms);
	while(QTime::currentTime() < dieTime)
		QApplication::instance()->processEvents();
}



#include "init.moc"

}}
