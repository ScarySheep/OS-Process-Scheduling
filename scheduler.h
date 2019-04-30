//#ifndef _SCHEDULER_H_
//#define _SCHEDULER_H_

#include "process.h"

#define FIFO 1
#define RR   2
#define SJF	 3
#define PSJF 4

//return next process
int next_process(struct process *proc, int nproc, int policy);

//running scheduler
int scheduling(struct process *proc, int nproc, int policy);

//#endif
