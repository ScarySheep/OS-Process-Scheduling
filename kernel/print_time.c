#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

asmlinkage void sys_print_time(pid_t pid,struct timespec *ts_start,struct timespec *ts_end){
	printk("[Project 1]%d %ld.%ld %ld.%ld\n",pid,ts_start->tv_sec,ts_start->tv_nsec,ts_end->tv_sec,ts_end->tv_nsec);
	return;
}
