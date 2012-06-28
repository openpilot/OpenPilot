/**
 * This is a small test program that connects to OpenPilot/CC via USB and writes a .opl compatible stream to stdout
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>


static inline int64_t timediff(struct timeval * old, struct timeval * new) {
	static int64_t usecs;
	usecs=new->tv_usec-old->tv_usec;
	usecs=usecs+((new->tv_sec-old->tv_sec)*1000000);
	return usecs;
}

// globals
int sock;
struct sockaddr_in remote;

static unsigned long granularity;

void findgranularity() {
	static struct timeval starttime,endtime;
	struct timespec tx;
	int t;
	gettimeofday(&starttime,NULL);
	for (t=0;t<10;t++) {
		tx.tv_sec=0;
		tx.tv_nsec=1;
		nanosleep(&tx,NULL);
	}
	gettimeofday(&endtime,NULL);
	granularity = timediff(&starttime,&endtime)/7;
}

static inline void dosleep(struct timeval * starttime, unsigned long sleeptime) {
	// sleep for the remaining time (if any)
	// if remaining time is smaller than  granularity
	// we are evil and do busy waiting
	struct timeval ctime;

	static struct timespec x;
	static unsigned long timed;

	gettimeofday(&ctime,NULL);
	timed=timediff(starttime,&ctime);
	while (timed<sleeptime) {
		if (sleeptime-timed>granularity) {
			x.tv_sec=0;
			x.tv_nsec=granularity*1000/2;
			nanosleep(&x,NULL);
		}
		gettimeofday(&ctime,NULL);
		timed=timediff(starttime,&ctime);
	}

}


static inline ssize_t mytf(int fd, void* buf, size_t count,ssize_t (*tff)(int,void*,size_t)) {

	ssize_t tf=0;
	while (count>0) {
		ssize_t res = (*tff)(fd,buf,count);
		if (res>0) {
			count-=res;
			buf+=res;
			tf+=res;
		} else {
			if (read<=0) return res;
			return tf;
		}
	}
	return tf;
}
static inline ssize_t myread(int fd, void* buf, size_t count) {
	return mytf(fd,buf,count,&read);
}
ssize_t xwrite(int fd, void* buf, size_t count) {
	return write(fd,buf,count);
}
static inline ssize_t mywrite(int fd, void* buf, size_t count) {
	return mytf(fd,buf,count,&xwrite);
}




int main() {

	struct timeval starttime,ctime;
	gettimeofday(&starttime,NULL);

	findgranularity();

	char ip[]="127.0.0.1";
	uint32_t port=9002;
	char logfile[]="/dev/stdin";

	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset (&remote,0,sizeof(remote));
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(ip);
	remote.sin_port = htons(port);
	
	int res = connect(sock, (struct sockaddr*)&remote,sizeof(remote));

	if (res!=0) {
		perror("failed to connect :");
		return 1;
	}
	printf("connected\n");


	int input = open(logfile,O_RDONLY);

	char buffer[65535];
	int n=0;
	while ((n = myread(input,buffer,sizeof(uint32_t)))>=0) {
		uint32_t timestamp = *((uint32_t*) buffer);
		n = myread(input,buffer,sizeof(uint64_t));
		if (n<=0) return(0);
		uint64_t dataSize = *((uint64_t*) buffer);
		n = myread(input,buffer,dataSize);
		if (n<=0) return(0);

		dosleep(&starttime,timestamp*1000);

		mywrite(sock,buffer,dataSize);

	}
	fprintf(stderr,"aborting: %s\n",strerror(-n));

}




