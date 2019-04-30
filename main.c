#define _GNU_SOURCE //打開cpu_set等功能 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "process.h"
#include "scheduler.h"

int main(int argc, char *argv[]){
	char policy[10];
	int N;
	
	scanf("%s",policy);
	scanf("%d",&N);
	
	struct process *proc = (struct process *)malloc(N *sizeof(struct process));
	
	for( int i = 0; i < N; i++ ){
		scanf("%s%d%d",proc[i].name,&proc[i].R,&proc[i].T);	
	}
	if( strcmp(policy,"FIFO") == 0 )
		scheduling(proc,N,FIFO);
	else if( strcmp(policy,"RR") == 0 )
		scheduling(proc,N,RR);
	else if( strcmp(policy,"SJF") == 0 )
		scheduling(proc,N,SJF);
	else if( strcmp(policy,"PSJF") == 0 )
		scheduling(proc,N,PSJF);
	else
		fprintf(stderr, "Wrong Input\n");
	exit(0);
} 
