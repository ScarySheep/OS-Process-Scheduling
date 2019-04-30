#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>

#define PARENT_CORE 0
#define CHILD_CORE 1

//Unit time.
#define UNIT_T() 									\
{													\
	for( volatile long i = 0; i < 1000000UL; i++ );	\
}													\

//Process structure.
struct process {
	char name[33];
	pid_t pid;
	int R;
	int T;
};

//Assign process to a particular cpu.
int proc_assign_cpu( int pid, int cpu );
//Execute the process.
int proc_exec( struct process proc );

int commence(int pid);
int Pause(int pid);

#endif
