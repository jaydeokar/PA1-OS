#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>


void refresh_epoch(){
	// Ignoring the null process thats at x = 0;
	
	int x = 1;
	while(x < NPROC){
		if (proctab[x].pstate != PRFREE){
			kprintf("before refresh sprio: %d,counter %d, gvalue %d\n",proctab[x].sprio,proctab[x].counter,proctab[x].gvalue);
			proctab[x].sprio = proctab[x].pprio;
			proctab[x].counter = (proctab[x].counter/2) + proctab[x].sprio;          // since a new process will have counter = 0, so no if else
			proctab[x].gvalue = proctab[x].counter + proctab[x].sprio;
			kprintf("After refresh sprio: %d,counter %d, gvalue %d\n",proctab[x].sprio,proctab[x].counter,proctab[x].gvalue);
		}
		x = x+1;
	}
}
