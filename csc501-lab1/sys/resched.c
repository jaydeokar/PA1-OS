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
	
	//kprintf("current pid %d \n",currpid);

	if (sel_scheduler == EXPDISTSCHED){
		// set the current pid and insert in the ready queue if the process is still in PRCURR
		// This should insert PRNULL at the beginning as no process is scheduled

		optr= &proctab[currpid];
		int rand;
		int left,right,lastMatch;

		if (optr->pstate == PRCURR) {
                        optr->pstate = PRREADY;
                        insert(currpid,rdyhead,optr->pprio);
                }
		
		rand = (int) expdev(0.1);
	//	kprintf("random value %d \n",rand);
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
//			kprintf("process prio: %d \n",q[lastMatch].qkey);
		}
		// this case happens when there is no null process in the ready queue. So the lastmatch doesn't get any process
		// and hence we need to execute null process
		if(right > NPROC){
                        currpid = 0;
                //        kprintf("right is greater than NPROC \n");
                }
		else{

//			kprintf("the process priority selected: %d \n",q[lastMatch].qkey);
			currpid = dequeue(lastMatch); 
		}
	 	
		nptr = &proctab[currpid];
                nptr->pstate = PRCURR;          /* mark it currently running    */
        #ifdef  RTCLOCK
                preempt = QUANTUM;              /* reset preemption counter     */
        #endif

                ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                /* The OLD process returns here when resumed. */
                return OK;

	}
	else if(sel_scheduler == LINUXSCHED){
		
		optr = &proctab[currpid];
		optr->counter  = preempt;
//		kprintf("pid: %d \n",currpid);

		if (currpid == 0){
			optr->counter = 0;
			optr->gvalue = 0;
		}
		else{
			if (preempt <= 0){
				optr->gvalue = 0;
				optr->counter = 0;
			}
			else{
				optr->gvalue = optr->sprio + optr->counter;
			}
		}

		//Step1: Epoch needs to be refreshed, when all runnable process have used up their time.
		int i = 1;
		for(;i < NPROC; i++){
			if( proctab[i].counter > 0 && (proctab[i].pstate == PRREADY || proctab[i].pstate == PRCURR)){
				break;
			}
		}

		if (i >= NPROC){
		        int x = 1;
        		while(x < NPROC){
                		if (proctab[x].pstate != PRFREE){
                        		proctab[x].sprio = proctab[x].pprio;
                        		proctab[x].counter = (proctab[x].counter/2) + proctab[x].sprio;  // new process will have counter = 0, so no if else
                        		proctab[x].gvalue = proctab[x].counter + proctab[x].sprio;
                		}
                		x = x+1;
        		}
	
		}
		//Step2: Find the greatest goodness value from the 
		// Goodness value will be from all the runnable process. this should be from the queue for RR
		// Iterate over the entire queue and get the maximum goodness from the queue from the tail
		
		int left = rdyhead;
                int right = q[rdytail].qprev;
		int maxGoodness = 0;
		int maxGoodnessProcess;
		while(left!=right){
			if (proctab[right].gvalue > maxGoodness && proctab[right].counter > 0){
				maxGoodness = proctab[right].gvalue;
				maxGoodnessProcess = right;
			}
			right = q[right].qprev;
		}
		// if the current goodness is greater than the max goodness obtained, run the current process given its state is still READY
		// else put the maximum goodness
		
		if (maxGoodness < optr->gvalue && optr->counter > 0){
			maxGoodnessProcess = currpid;
			maxGoodness = optr->gvalue;
		}
	
               if (optr->pstate == PRCURR) {
                		optr->pstate = PRREADY;
                        	insert(currpid,rdyhead,optr->pprio);
                }
		

//		if (maxGoodnessProcess == 0){
		//	kprintf("null process started \n");
//			currpid = NULLPROC;
//			nptr = &proctab[currpid];
 //               	nptr->pstate = PRCURR;          /* mark it currently running    */
//        	#ifdef  RTCLOCK
//                	preempt = QUANTUM;              /* reset preemption counter     */
//        	#endif
//                	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

                /* The OLD process returns here when resumed. */
//                	return OK;

//		}

//		else{
				
			currpid = dequeue(maxGoodnessProcess);
			nptr = &proctab[currpid];
                        nptr->pstate = PRCURR;          /* mark it currently running    */
                #ifdef  RTCLOCK
                        preempt = nptr->counter;              /* reset preemption counter     */
                #endif
                        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
			return OK;
		
//	}


		//Step3: If the max value is still 0, then run the null process. Check if the current one is null process
		//if the current is null return ok

		//Step4: If the max value > 0, the process is scheduled and the preemption time is st			


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
