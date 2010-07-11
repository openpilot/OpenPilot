/**
 * small test program whether signals between threads work as they should
 */

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

void sighandler(int sig) {
	write(2,".",1);
	return;
}

void* threadstart(void* arg) {
	char buf[1024];

	while (1) {
		read(1,buf,512);
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
