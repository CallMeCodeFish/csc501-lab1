#ifndef _SCHED_H_
#define _SCHED_H_

#define DEFAULT 0
#define EXPDISTSCHED 1
#define LINUXSCHED 2
#define LAMBDA 0.1

void setschedclass(int sched_class);

int getschedclass();

int getnext(int key);

int reverseinsert(int proc, int head, int key);

void clearqueue();

#endif