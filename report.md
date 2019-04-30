[OS] Process Scheduling
===

## 設計

### Makefile
---

在directory輸入以下指令
* **make** - Compile all the codes
* **make clean** - Delete all .o files
* **make run** - sudo ./project will be executed

在kernel資料夾裡，有兩個c檔**print_time.c**與**get_nstime.c** 需要放進linux/kernel資料夾

**Syscalls number**
* **334** - sys_print_time()
* **335** -  get_nstime.c



### Main
---

Main.c的主要功能為讀取測資，決定要用甚麼Policy，每個Process的Ready Time以及Execution Time。再將這些資訊以struct process傳遞給Scheduler.c做處理 
```C
#define _GNU_SOURCE 
```
程式碼最上面這行讓我們可以打開CPU_SET等功能
```C
#include "process.h"
#include "scheduler.h"
```
include我們其他檔案的標頭檔


### Scheduler
---
從Main過來首先進入
```C
int scheduling(struct process *proc, int number, int policy)
{
    qsort(proc, number, sizeof(struct process), comp);
    ......
}
```
Quicksort的用途為讓程式由Ready Time小的開始做
```C
    proc_assign_cpu(getpid(), PARENT_CORE);
    commence(getpid());
```
第一行的目的為設定單一核心避免搶占(在使用虛擬機時要先設定為雙核，跑這行程式才不會出錯)
第二行為設定Scheduler高優先度

接著我們進入一個While迴圈，這個迴圈的作用為切換不同的Process
首先透過next_process獲取不同policy情況下下個執行的process，並且藉由process.c的commence及pause function開始以及暫停process
```C
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
```
再來是檢查Process是否ready然後準備被執行

```C
for (int i = 0; i < number; i++) {
    if (proc[i].R == current_unit) {
        proc[i].pid = proc_exec(proc[i]);
        Pause(proc[i].pid);
    }
}
```
以及計算單位時間
```C
UNIT_T();
if (idofrp != -1)
    proc[idofrp].T--;
current_unit++;
```
最後如果所有的Process都結束了就跳出迴圈
```C
if (idofrp != -1 && proc[idofrp].T == 0) {
    waitpid(proc[idofrp].pid, NULL, 0);
    printf("%s %d\n", proc[idofrp].name, proc[idofrp].pid);
    idofrp = -1;
    finish++;

    if (finish == number)
	break;
}

```
剛剛的While迴圈切換process時會呼叫下列function
```C
int next_process(struct process *proc, int number, int policy)
```
function內部為巢狀if else，分類PSJF、SJF、FIFO以及RR
如果今天有process已經在執行了，且policy為FIFO以及SJF，因為是non-preemptive就直接return
```C
if(idofrp != -1 && (policy == 1 || policy == 3))
	return idofrp;
```
**PSJF或SJF**
比較execution的長短
```C
for (int i = 0; i < number; i++) {
    if (proc[i].pid == -1 || proc[i].T == 0)
        continue;
    if (now == -1 || proc[i].T < proc[now].T)
        now = i;
}
```

**PSJF或SJF**
比較execution的長短
```C
for (int i = 0; i < number; i++) {
    if (proc[i].pid == -1 || proc[i].T == 0)
        continue;
    if (now == -1 || proc[i].T < proc[now].T)
        now = i;
}
```

**FIFO**
比較ready time的先後
```C
for(int i=0; i<number; i++){
    if(proc[i].pid == -1 || proc[i].T ==0)
        continue;
    if(now == -1 || proc[i].R < proc[now].R)
        now = i;
}
```
**RR**
固定時間到了就切換下一個process
```C
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
else{
    now = idofrp;
}
```
### Process
---
Process.c的作用為提供剛剛兩個程式所需對cpu執行的函數
首先proc_assign_cpu在scheduling function的開頭有用到，用作將process指派到特定CPU
```C
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
```
proc_exec function作用為執行process
```C
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
```

這兩個function目的為開始與暫停特定process，透過增加或是減少process之間相對的priority來達成這個作用，commence以及pause的差別在於一個使用SCHED_OTHER另一個則是SCHED_IDLE
```C
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
```

執行範例測資
---

### **FIFO**
---
![](https://i.imgur.com/5leEjnA.png)

![](https://i.imgur.com/WBrH3c7.png)

![](https://i.imgur.com/Ru65723.png)

![](https://i.imgur.com/ogdytZD.png)

![](https://i.imgur.com/k97BA3o.png)

### **PSJF**
---
![](https://i.imgur.com/sCRwoMs.png)

![](https://i.imgur.com/gGL9PRA.png)

![](https://i.imgur.com/Rq9Dbuy.png)

![](https://i.imgur.com/XJY7VvC.png)

![](https://i.imgur.com/bxFX2MF.png)

### **RR**
---
![](https://i.imgur.com/lH84J22.png)

![](https://i.imgur.com/0SU8QCp.png)

![](https://i.imgur.com/rnudqrb.png)

![](https://i.imgur.com/rDVaCwn.png)

![](https://i.imgur.com/JvOk5r8.png)

### **SJF**
---
![](https://i.imgur.com/4xshLX5.png)
![](https://i.imgur.com/dv14OX5.png)
![](https://i.imgur.com/eiDH8IZ.png)
![](https://i.imgur.com/Io5QIMh.png)
![](https://i.imgur.com/ajydFyp.png)


## 結果與討論

### 輸出不一致
以FIFO舉例，輸出的測資執行時間、pid等資訊是正確的，但是輸出並不是按照順序
如FIFO 2測資的輸出:
```C
2408 1556540107.992360657    1556540262.602365500
2409 1556540110.992360657    1556540262.333742383
2411 1556540110.992360657    1556540262.207245489
2410 1556540110.992360657    1556540262.245464779
```
結論是C的qsort會有unstable的情況，才導致這樣的情況發生

以下是四種Policy跑範例測資的結果
**FIFO**
```C
P1	0	500
P2	0	500
P3	0	500
P4	0	500
P5	0	500
```
![](https://i.imgur.com/Hup8LgN.png)

**RR**
```C
P1	0	500
P2	0	500
P3	0	500
P4	0	500
P5	0	500
```
![](https://i.imgur.com/0MaIg4G.png)

**PSJF**
```C
P1	0	10000
P2	3000	7000
P3	2000	5000
P4	3000	3000
```
![](https://i.imgur.com/WShoimz.png)

**SJF**
```C
P1	0	7000
P2	0	2000
P3	100	1000
P4	200	4000
```
![](https://i.imgur.com/5kRk2Ne.png)

因為RR、PSJF等Policy starting time並不明確，我們這邊都使用ready time作為輸出時間，可以看到表格所繪出的時間軸皆正確，但是看到如FIFO的範例，每個process執行的時間還是有些許的誤差，原因可能是因為電腦cpu的差異，有些cpu會在特定情況進行加速(如intel的turbo boost)，因此我們使用的，透過跑for迴圈來計時的方式就會產生些許誤差。

各組員的貢獻
---
* 許定為 - Main&MakeFile + 統整程式碼 + Debug
* 吳綺緯 廖晨皓 - Process 
* 楊晨弘 徐浩翔 - Scheduling
* 李正己 - Report + 結果分析