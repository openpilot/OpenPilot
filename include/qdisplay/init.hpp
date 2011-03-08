#ifndef _QDISPLAY_DISPLAYINIT_H_
#define _QDISPLAY_DISPLAYINIT_H_

#include <QObject>
#include <QApplication>
#include <QTimer>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include "kernel/threads.hpp"

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
		void (*onExit_)(SharedDataStructure*,boost::thread*);
		boost::thread *thread_main;
		QApplication *app;
		QTimer *timer;
		SharedDataStructure *sharedDataStructure;
		
	public:
		/**
		If you want a single thread, only specify display, otherwise main and display will run
		in parallel (they can create other threads), communicate data with sharedDataStructure
		and must synchronize using a kernel::FifoMutex in sharedDataStructure.
		
		If you want the display task to be executed periodically, specify diplay_interval (ms),
		otherwise it will be called only once, but the display task must execute
		"QApplication::instance()->processEvents();" regularly so that the display is refreshed.
		Moreover if you want to sleep or lock a mutex, you should use qtSleep and qtMutexLock
		functions so that the display is refreshed.
		
		@param _display the function that does the display. Must return void and accept
		one parameter which is a pointer, and must be cast to qdisplay::FUNC.
		@param prioDisplay the priority of the display thread (unix process priority : -20 (most priority) to 20
		@param _main the optional function that executes the main program in another thread (see _display)
		@param prioMain the priority of the main thread (see prioDisplay)
		@param display_interval the interval in ms at which the display function is called. 0 means only once.
		@param _sharedDataStructure the data structure that is passed to _main and _display functions
		@param _onExit the optional function that is executed just before the application terminates because all windows are closed.
		Must return void and accept two parameters, a void pointer and a pointer to the main thread, to have the possibility to join
		it. The function must be cast to qdisplay::EXIT_FUNC
		*/
		QtAppStart(void (*_display)(SharedDataStructure*), int prioDisplay = 0,
			void (*_main)(SharedDataStructure*) = NULL, int prioMain = 0,
			int display_interval = 0, SharedDataStructure *_sharedDataStructure = NULL, 
			void (*_onExit)(SharedDataStructure*,boost::thread*) = NULL);
		~QtAppStart();
		
	public slots:
		void display();
		void onExit();
};

/**
The function pointer type you must cast to the functions you give as a parameter to QtAppStart
*/
typedef void(*FUNC)(void*);
typedef void(*EXIT_FUNC)(void*,boost::thread*);

/**
Special sleep function that processes qt events to refresh display
@param ms sleep time in milliseconds
*/
void qtSleep(int ms);

/**
Special lock function that processes qt events to refresh display
@param mutex the mutex to lock, that must provide a try_lock implementation
*/
template<class Mutex>
void qtMutexLock(Mutex &mutex)
{
	while (!mutex.try_lock())
		QApplication::instance()->processEvents();
}

}}

#endif
