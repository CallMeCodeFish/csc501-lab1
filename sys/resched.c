/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>
#include <math.h>

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

	int schedclass = getschedclass();

	if (schedclass == EXPDISTSCHED) {
		//exponential distribution scheduler
		optr = &proctab[currpid];

		if (optr->pstate == PRCURR) {
			optr->pstate = PRREADY;
			reverseinsert(currpid, rdyhead, optr->pprio);
		}

		int minpriority = (int) expdev(LAMBDA);

		currpid = getnext(minpriority);

		nptr = &proctab[currpid];

		// kprintf("\npid = %d", currpid);

		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	} else if (schedclass == LINUXSCHED) {
		//Linux-like scheduler
		optr = &proctab[currpid];

		int numticks = 1 - preempt;
		if (numticks < 0) {
			numticks = 0;
		}

		if (currpid != NULLPROC) {
			if (numticks > optr->counter) {
				numticks = optr->counter;
			}
			optr->counter -= numticks;
		}

		epoch -= numticks;

		if (epoch <= 0) {
			
			clearqueue();
			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
			}
			
			epoch = 0;
			int i;
			for (i = 0; i < NPROC; ++i) {
				if ((&proctab[i])->pstate != PRFREE) {
					struct pentry *tmp = &proctab[i];
					tmp->curprio = tmp->pprio;
					tmp->counter = tmp->counter / 2 + tmp->curprio;
					epoch += tmp->counter;
					if (i == 0 || tmp->pstate == PRREADY) {
						int goodness = tmp->counter + tmp->curprio;
						insert(i, rdyhead, goodness);
					}
				}
			}

			// kprintf("\nnew epoch = %d\n", epoch);

			currpid = getlast(rdytail);
			nptr = (&proctab[currpid]);
		} else {
			// kprintf("\nold epoch\n");

			// if (q[rdytail].qprev == 48) {
			// 	kprintf("\nvalue = %d\n", epoch);
			// }

			int goodness = 0;
			if (optr->counter > 0) {
				goodness = optr->counter + optr->curprio;
			}

			if (optr->pstate == PRCURR && optr->counter > 0 && goodness > lastkey(rdytail)) {
				preempt = 1;
				return OK;
			}
			
			if (optr->pstate == PRCURR && (currpid == NULLPROC || optr->counter > 0)) {
				optr->pstate = PRREADY;
				insert(currpid, rdyhead, goodness);
			}

			currpid = getlast(rdytail);
			nptr = (&proctab[currpid]);
		}

		#ifdef RTCLOCK
			preempt = 1;
		#endif
		
	} else {
		//default scheduling policy provided by XINU

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

		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}

	nptr->pstate = PRCURR;		/* mark it currently running	*/
	

	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
