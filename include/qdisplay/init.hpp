#ifndef _QDISPLAY_DISPLAYINIT_H_
#define _QDISPLAY_DISPLAYINIT_H_

#include <QObject>
#include <QApplication>
#include <QTimer>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

namespace jafar {
namespace qdisplay {

/**
	This class provides a simple way to control a qt display from c++,
	dealing with all the constraints of qt.
	Cannot use a template type because it is incompatible with Qt,
	so use a void* for the shared data structure...
*/
//template<typename SharedDataStructure = void>
class QtAppStart: public QObject
{
	Q_OBJECT
	private:
		typedef void SharedDataStructure; // cannot use templates with q_object... sucks
		void (*main_)(SharedDataStructure*);
		void (*display_)(SharedDataStructure*);
		boost::thread *thread_main;
		QApplication *app;
		QTimer *timer;
		SharedDataStructure *sharedDataStructure;
		
	public:
		/**
		If you want a single thread, only specify display, otherwise main and display will run
		in parallel (they can create other threads), communicate data with sharedDataStructure
		and must synchronize using a boost::shared_mutex (.lock_shared and .unlock_shared) in sharedDataStructure.
		
		If you want the display task to be executed periodically, specify diplay_interval (ms),
		otherwise it will be called only once, but the display task must execute
		"QApplication::instance()->processEvents();" regularly so that the display is refreshed.
		
		@param _display the function that does the display
		@param _main the optional function that executes the main program in another thread
		@param display_interval the interval in ms at which the display function is called. 0 means only once.
		@param _sharedDataStructure the data structure that is passed to _main and _display functions
		*/
		QtAppStart(void (*_display)(SharedDataStructure*),
			void (*_main)(SharedDataStructure*) = NULL, 
			int display_interval = 0, SharedDataStructure *_sharedDataStructure = NULL);
		~QtAppStart();
		
	public slots:
		void display();
		
};
	

typedef void(*FUNC)(void*);

void qtSleep(int ms);

}}

#endif
