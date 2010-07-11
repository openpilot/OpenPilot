/**
 * small etst program whether signals between threads work as they should
 */

#include <time.h>
#include <pthread.h>
#include <signal.h>


/**
 * actual test program
 */

void sighandler(int sig) {
	return;
}


void* threadstart(void* arg) {
struct timespec timeout;	
int t;

	while (1) {
		timeout.tv_sec=0;
		timeout.tv_nsec=1;
		nanosleep(&timeout,0);
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
