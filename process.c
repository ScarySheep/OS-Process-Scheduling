#define _GNU_SOURCE
#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>

#define PRINTK 334
#define GET_TIME 335
//Assign process to particular core.
int proc_assign_cpu( int pid, int cpu )
{
	if( cpu > sizeof( cpu_set_t ) ){
		printf( "Error: Assign to wrong CPU." );
		return -1;
	}	

	cpu_set_t cpu_assign;
	CPU_ZERO( &cpu_assign );
	CPU_SET( cpu, &cpu_assign );

	if( sched_setaffinity( pid, sizeof( cpu_assign ), &cpu_assign ) < 0 ){
		printf( "Error: Set process affinity error." );
		exit( 1 );
	}

	return 0;
}
//Execute the process.
int proc_exec( struct process proc )
{
	int pid = fork();
	
	if( pid < 0 ){
		printf( "Error: Fork error." );
		return -1;
	}
	if( pid == 0 ){	//Child process.
		struct timespec ts_start, ts_end;
		syscall( GET_TIME, &ts_start);
		for( int i = 0; i < proc.T; i++ ){
			UNIT_T();
		}
		syscall( GET_TIME, &ts_end);
		syscall( PRINTK, getpid(), &ts_start, &ts_end );
		exit( 0 );
	}

	proc_assign_cpu( pid, CHILD_CORE );
	
	return pid;
}

//commence and pause are functions to start and stop a given process
//we achieve this by increasing or lowering,respectively the priority of the process
//
int commence(int pid){
	struct sched_param process;
	process.sched_priority = 0;
	
	
	int result = sched_setscheduler(pid,SCHED_OTHER, &process);
	if(result < 0){
		perror("sched_setscheduler");
		return -1;
	}
	return result;
}

//we do the same as previous except its sched_idle instead of sched_other
int Pause(int pid){
	struct sched_param process;
	process.sched_priority = 0;
	
	
	int result = sched_setscheduler(pid,SCHED_IDLE, &process);
	if(result < 0){
		perror("sched_setscheduler");
		return -1;
	}
	return result;
}
