/**
 * small etst program whether signals between threads work as they should
 */

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

static pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t pfMutex = PTHREAD_MUTEX_INITIALIZER;

#define printf(...) (void)pthread_mutex_lock(&pfMutex);printf(__VA_ARGS__);(void)pthread_mutex_unlock(&pfMutex)

void sighandler(int sig) {
	printf("signal handler called in thread %li - signal %i\n",(long)pthread_self(),sig);
}


void* threadstart(void* arg) {
struct timespec timeout;	
int t;
	printf("thread %li started\n",(long)pthread_self());

	while (1) {
		printf("getting mutex\n");
		pthread_mutex_lock(&Mutex);
		printf("got mutex\n");


		for (t=0;t<20;t++) {
			timeout.tv_sec=1;
			timeout.tv_nsec=0;
			nanosleep(&timeout,0);
			printf("thread %li still running...\n",(long)pthread_self());
		}
		pthread_mutex_unlock(&Mutex);
		sleep(1);
	}
}


int main(char** argc, int argv) {

	pthread_t testthread1;	
	pthread_t testthread2;	
	struct sigaction action;
	printf("thread test program\n");
	printf("installing signal handler\n");
	action.sa_handler=sighandler;
	action.sa_flags=0;
	sigfillset( &action.sa_mask );
	sigaction(SIGUSR1,&action,NULL);

	sleep(5);
	printf("starting thread 1\n");
	pthread_create(&testthread1,NULL,threadstart,NULL);
	sleep(5);
	printf("starting thread 2\n");
	pthread_create(&testthread2,NULL,threadstart,NULL);
	while (1) {
		sleep(5);
		printf("sending SIG_USR1 to thread 1\n");
		pthread_kill(testthread1,SIGUSR1);
		sleep(5);
		printf("sending SIG_USR1 to thread 2\n");
		pthread_kill(testthread2,SIGUSR1);
	}
}
