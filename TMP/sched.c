#include <sched.h>
#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <proc.h>

volatile int sched_policy = DEFAULT;

void setschedclass(int sched_class) {
	if (sched_class == LINUXSCHED) {
		STATWORD ps;
		disable(ps);
		sched_policy = sched_class;
		resched();
		restore(ps);
	} else {
		sched_policy = sched_class;
	}
}

int getschedclass() {
    return sched_policy;
}

int getnext(int key) {
    int proc = q[rdyhead].qnext;

    while (q[proc].qnext != rdytail && q[proc].qkey <= key)
        proc = q[proc].qnext;

	if (proc < NPROC) {
		// kprintf("\nif: %d\n", proc);
		return( dequeue(proc) );
	}
	else {
		// kprintf("\nelse: %d\n", proc);
		return(EMPTY);
	}
}

int reverseinsert(int proc, int head, int key) {
    int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	while (q[next].qkey <= key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;
	q[prev].qnext = proc;
	q[next].qprev = proc;
	return(OK);
}

void clearqueue() {
	q[rdyhead].qnext = rdytail;
	q[rdytail].qprev = rdyhead;
}
