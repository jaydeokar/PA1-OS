/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>

unsigned long currSP;	/* REAL sp of current process */
extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	if (sel_scheduler == EXPDISTSCHED){
		// set the current pid and insert in the ready queue if the process is still in PRCURR
		optr= &proctab[currpid];
		int rand;
		int left,right,lastMatch;

		if (optr->pstate == PRCURR) {
                        optr->pstate = PRREADY;
                        insert(currpid,rdyhead,optr->pprio);
                }
		
		rand = (int) expdev(0.1);
		kprintf("random value %d \n",rand);	
		right = q[rdytail].qprev;
 		left = q[rdyhead].qnext;  
		lastMatch = right;

		
		while( q[right].qkey > rand && (left != right && right < NPROC)) {

			// to pick the first one from tail in case of equal prio

			if (q[lastMatch].qkey != q[right].qkey)
				lastMatch = right;

			right = q[right].qprev;

			if ( right == left && q[right].qkey > rand) {
				if (q[lastMatch].qkey != q[right].qkey){
                                	lastMatch = right;
				}
			}
		}

	 	nptr = &proctab[ (currpid = dequeue(lastMatch)) ];
                nptr->pstate = PRCURR;          /* mark it currently running    */
        #ifdef  RTCLOCK
                preempt = QUANTUM;              /* reset preemption counter     */
        #endif

                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                /* The OLD process returns here when resumed. */
                return OK;

	}
	else if(sel_scheduler == LINUXSCHED){
	


	}
	else{
        	/* no switch needed if current process priority higher than next*/
		if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   	(lastkey(rdytail)<optr->pprio)) {
			return(OK);
		}
	
		/* force context switch */

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			insert(currpid,rdyhead,optr->pprio);
		}	

		/* remove highest priority process at end of ready list */

		nptr = &proctab[ (currpid = getlast(rdytail)) ];
		nptr->pstate = PRCURR;		/* mark it currently running	*/
	#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
	#endif
	
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
		/* The OLD process returns here when resumed. */
		return OK;
	}
}
