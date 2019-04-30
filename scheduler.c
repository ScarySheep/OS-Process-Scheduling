#define _GNU_SOURCE
#include "process.h"
#include "scheduler.h"
#include<signal.h>
#include<stdlib.h>
#include<stdio.h>
#include<sched.h>
#include<sys/wait.h>
#include<unistd.h>

static int lastcs; //last context switch time for RR
static int current_unit; //current unit time
static int idofrp; //id of running process. -1 if no process running
static int finish; //the number of finish process

int comp(const void *a, const void *b){
	return ((struct process *)a)->R - ((struct process *)b)->R;
}

int next_process(struct process *proc, int number, int policy){
	if(idofrp != -1 && (policy == FIFO || policy == SJF))
		return idofrp;

	int now = -1;
	if (policy == PSJF || policy ==  SJF) {
		for (int i = 0; i < number; i++) {
			if (proc[i].pid == -1 || proc[i].T == 0)
				continue;
			if (now == -1 || proc[i].T < proc[now].T)
				now = i;
		}
	}
	else if(policy == FIFO){
		for(int i=0; i<number; i++){
			if(proc[i].pid == -1 || proc[i].T ==0)
				continue;
			if(now == -1 || proc[i].R < proc[now].R)
				now = i;
		}
	}
	else if (policy == RR){
		if(idofrp == -1){
			for(int i=0; i<number; i++){
				if(proc[i].pid != -1 && proc[i].T > 0){
					now = i;
					break;
				}
			}
		}
		else if((current_unit - lastcs) % 500 == 0){
			now = (idofrp + 1) % number;
			while(proc[now].pid == -1 || proc[now].T == 0)
				now = (now + 1) % number;
		}
		else
			now = idofrp;
	}
	return now;
}

int scheduling(struct process *proc, int number, int policy)
{
	qsort(proc, number, sizeof(struct process), comp);
	//由ready time 小的先開始做



	/* Initial pid = -1 imply not ready */
	for (int i = 0; i < number; i++)
		proc[i].pid = -1;

	/* Set single core prevent from preemption */
	proc_assign_cpu(getpid(), PARENT_CORE);

	/* Set high priority to scheduler */
	commence(getpid());

	/* Initial scheduler */
	current_unit = 0;
	idofrp = -1;
	finish = 0;

	while(1) {
		//fprintf(stderr, "Current time: %d\n", ntime);

		/* Check if running process finish */
		if (idofrp != -1 && proc[idofrp].T == 0) {

#ifdef DEBUG
			fprintf(stderr, "%s finish at time %d.\n", proc[idofrp].name, current_unit);
#endif
			//kill(idofrp, SIGKILL);
			waitpid(proc[idofrp].pid, NULL, 0);
			printf("%s %d\n", proc[idofrp].name, proc[idofrp].pid);
			idofrp = -1;
			finish++;

			/* 所有process都完成了結束while*/
			if (finish == number)
				break;
		}

		/* Check if process ready and execute */
		for (int i = 0; i < number; i++) {
			if (proc[i].R == current_unit) {
				proc[i].pid = proc_exec(proc[i]);
				Pause(proc[i].pid);
#ifdef DEBUG
				fprintf(stderr, "%s ready at time %d.\n", proc[i].name, current_unit);
#endif
			}

		}

		/* Select next running  process */
		int next = next_process(proc, number, policy);
		if (next != -1) {
			/* Context switch */
			if (idofrp != next) {
				commence(proc[next].pid);
				Pause(proc[idofrp].pid);
				idofrp = next;
				lastcs =current_unit;
			}
		}

		/* Run an unit of time */
		UNIT_T();
		if (idofrp != -1)
			proc[idofrp].T--;
		current_unit++;
	}

	return 0;
}
