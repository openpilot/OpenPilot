/**
 * small etst program whether signals between threads work as they should
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

static pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * printf is not thread safe - this little helper function allow debug output in thread safe context
 */
/* write a string to a filehandle */
void print_char(int fh, char *c) {
	int t=0;
	while (c[t]!=0) t++;
	write(fh,c,t);
}

/* create a dezimal string from an integer */
int int2char(char* x,unsigned long i) {
	if (i==0) {
		return 0;
	}
	int k=int2char(&x[0],i/10);
	x[k]='0'+(i%10);
	x[k+1]=0;
	return k+1;
}
/* print a number*/
void print_number(int fh, long i) {
	char buffer[39]; // 39 characters are enough to store a 128 bit integer in dezimal notation (~~3.4* 10^38)
	
	if (i==0) {
		print_char(fh,"0");
	} else {
		if (i<0) {
			buffer[0]='-';
			int2char(&buffer[1],-i);
		} else {
			int2char(buffer,i);
		}
		print_char(fh,buffer);
	}
}

/**
 * actual test program
 */

void sighandler(int sig) {
	print_char(2,"signal handler called in thread ");
	print_number(2,(long)pthread_self());
	print_char(2," - signal ");
	print_number(2,sig);
	print_char(2,"\n");
}


void* threadstart(void* arg) {
struct timespec timeout;	
int t;
	print_char(2,"thread ");
	print_number(2,(long)pthread_self());
	print_char(2," started \n");

	while (1) {
		print_char(2,"getting mutex\n");
		pthread_mutex_lock(&Mutex);
		print_char(2,"got mutex\n");


		for (t=0;t<20;t++) {
			timeout.tv_sec=1;
			timeout.tv_nsec=0;
			nanosleep(&timeout,0);
			
			print_char(2,"thread ");
			print_number(2,(long)pthread_self());
			print_char(2," still running...\n");
		}
		pthread_mutex_unlock(&Mutex);
		sleep(1);
	}
}


int main(char** argc, int argv) {

	pthread_t testthread1;	
	pthread_t testthread2;	
	struct sigaction action;
	print_char(2,"thread test program\n\n");

	print_char(2,"demonstrate print function\n");
	long t=1;
	while (t!=0) {
		print_number(2,t);
		print_char(2," >> ");
		t=(t>0)?t*2:t/2;
	}
	print_number(2,t);
	print_char(2,"\ndone\n\n");

	print_char(2,"installing signal handler\n");
	action.sa_handler=sighandler;
	action.sa_flags=0;
	sigfillset( &action.sa_mask );
	sigaction(SIGUSR1,&action,NULL);

	sleep(5);
	print_char(2,"starting thread 1\n");
	pthread_create(&testthread1,NULL,threadstart,NULL);
	sleep(5);
	print_char(2,"starting thread 2\n");
	pthread_create(&testthread2,NULL,threadstart,NULL);
	while (1) {
		sleep(5);
		print_char(2,"sending SIG_USR1 to thread 1\n");
		pthread_kill(testthread1,SIGUSR1);
		sleep(5);
		print_char(2,"sending SIG_USR1 to thread 2\n");
		pthread_kill(testthread2,SIGUSR1);
	}
}
