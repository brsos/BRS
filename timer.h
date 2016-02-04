
#ifdef _WIN32

#include<time.h>
double getTime(){return clock()/1000.0;}

#else

#include <sys/time.h>
double getTime(){
	static struct timeval tv;
	static struct timezone tz;
	gettimeofday(&tv, &tz);
	return tv.tv_usec/1000000.0+tv.tv_sec;
}
#endif


struct Timer{
	double acc;
	double last;
};
void reset(struct Timer* t){t->acc=0;}
void start(struct Timer* t){
	t->last=getTime();
}
void stop(struct Timer* t){
	double cur=getTime();
	t->acc+=cur-t->last;
	t->last=0;
}

