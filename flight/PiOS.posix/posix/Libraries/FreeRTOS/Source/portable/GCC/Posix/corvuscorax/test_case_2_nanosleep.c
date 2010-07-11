/**
 * small test program whether signals between threads work as they should
 */

#include <time.h>
#include <pthread.h>
#include <signal.h>

void sighandler(int sig) {
	write(2,".",1);
	return;
}

void* threadstart(void* arg) {
struct timespec sleeptime;

	while (1) {
		sleeptime.tv_sec=1;
		sleeptime.tv_nsec=0;
		nanosleep(&sleeptime,NULL);
	}
}

int main(char** argc, int argv) {

	pthread_t testthread1;	
	struct sigaction action;

	action.sa_handler=sighandler;
	action.sa_flags=0;
	sigfillset( &action.sa_mask );
	sigaction(SIGUSR1,&action,NULL);

	pthread_create(&testthread1,NULL,threadstart,NULL);
	while (1) {
		pthread_kill(testthread1,SIGUSR1);
	}
}
