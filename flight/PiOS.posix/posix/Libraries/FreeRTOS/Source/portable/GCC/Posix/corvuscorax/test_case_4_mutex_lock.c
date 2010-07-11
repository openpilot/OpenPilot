/**
 * small etst program whether signals between threads work as they should
 */

#include <pthread.h>
#include <signal.h>

static pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * actual test program
 */

void sighandler(int sig) {
	write(2,".",1);
	return;
}

void* threadstart(void* arg) {

	while (1) {
		pthread_mutex_lock(&Mutex);
	}
}

int main(char** argc, int argv) {

	pthread_t testthread1;	
	struct sigaction action;

	action.sa_handler=sighandler;
	action.sa_flags=0;
	sigfillset( &action.sa_mask );
	sigaction(SIGUSR1,&action,NULL);

	pthread_mutex_lock(&Mutex);
	pthread_create(&testthread1,NULL,threadstart,NULL);
	while (1) {
		pthread_kill(testthread1,SIGUSR1);
	}
}
