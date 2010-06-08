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

	QtAppStart::QtAppStart(void (*_display)(SharedDataStructure*),
		void (*_main)(SharedDataStructure*), 
		int display_interval, SharedDataStructure *_sharedDataStructure):
		main_(_main), display_(_display), thread_main(NULL), app(NULL), timer(NULL), 
		sharedDataStructure(_sharedDataStructure)
	{
		if (main_) thread_main = new boost::thread(boost::bind(main_,sharedDataStructure));
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
		if (thread_main && thread_main->timed_join(boost::posix_time::milliseconds (0))) app->exit();
		(*display_)(sharedDataStructure);
	}

void qtSleep(int ms)
{
	QTime dieTime = QTime::currentTime().addMSecs(ms);
	while(QTime::currentTime() < dieTime)
		QApplication::instance()->processEvents();
}

#include "init.moc"

}}
