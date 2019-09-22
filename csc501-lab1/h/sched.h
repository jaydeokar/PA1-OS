#ifndef _SCHED_H__
#define _SCHED_H__

#define EXPDISTSCHED 1
#define LINUXSCHED 2
#define HIGHPRIO 0



void setschedclass (int sched_class);
int getschedclass();


#endif

extern int sel_scheduler;

