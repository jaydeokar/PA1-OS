#include <kernel.h>
#include <math.h>
#include <sched.h>


int sel_scheduler;

void setschedclass (int sched_class){

	sel_scheduler = sched_class;

}


int getschedclass(){
	
	return sel_scheduler;
}

