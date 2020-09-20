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

int isfirst = 1;

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

		nptr->pstate = PRCURR;		/* mark it currently running	*/

		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	} else if (schedclass == LINUXSCHED) {
		//Linux-like scheduler
		optr = &proctab[currpid];
		
		if (isfirst == 1 || preempt < 0) {
			preempt = 0;
		}

		if (currpid == NULLPROC) {
			epoch--;
		} else {
			int used = optr->counter - preempt;
			if (used < 0) {
				used = 0;
			}
			epoch -= used;
			optr->counter = preempt;
		}

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
					// if (isfirst == 1 && i == 49) {
					// 	kprintf("\nmain: counter = %d, prio = %d\n", tmp->counter, tmp->pprio);
					// }
					tmp->curprio = tmp->pprio;
					tmp->counter = tmp->counter / 2 + tmp->curprio;
					// if (isfirst == 1 && tmp->counter > 0) {
					// 	kprintf("\npid: %d, counter: %d, state: %d\n", i, tmp->counter, tmp->pstate);
					// }
					epoch += tmp->counter;
					if (i == 0 || tmp->pstate == PRREADY) {
						int goodness = tmp->counter + tmp->curprio;
						insert(i, rdyhead, goodness);
					}
				}
			}
			if (isfirst == 1) {
				isfirst = 0;
				// kprintf("\nnew = %d\n", epoch);
			}

			currpid = getlast(rdytail);
			nptr = (&proctab[currpid]);

			// kprintf("\nnew = %d\n", epoch);
		} else {
			// kprintf("\nold = %d\n", epoch);

			if (optr->pstate == PRCURR && optr->counter > 0) {
				#ifdef RTCLOCK
					preempt = optr->counter;
				#endif
				return OK;
			}

			if (optr->pstate == PRCURR) {
				optr->pstate = PRREADY;
				if (currpid == NULLPROC) {
					insert(NULLPROC, rdyhead, 0);
				}
			}

			int goodness = 0;
			if (optr->counter > 0) {
				goodness = optr->counter + optr->curprio;
			}

			currpid = getlast(rdytail);
			nptr = (&proctab[currpid]);
		}
		nptr->pstate = PRCURR;		/* mark it currently running	*/
		#ifdef RTCLOCK
			if (currpid == NULLPROC) {
				preempt = 1;
			} else {
				preempt = nptr->counter;
			}
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

		nptr->pstate = PRCURR;		/* mark it currently running	*/

		#ifdef	RTCLOCK
			preempt = QUANTUM;		/* reset preemption counter	*/
		#endif
	}

	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
