#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TIMEQUANTUM 5
#define MAXTIME 100

typedef struct tagProcess{           
    int pid; 
    //초반에 정해줘야 하는 것들 
    int CPU_burst_time;
    int IO_burst_time;
    int Arrival_time;
    int Priority;

    int CPU_burst_remain;
    int IO_burst_remain;
    int IO_occuring_time; 
    int TimeInRunning;

    int Waiting_time;
    int TurnAround_time; 
    int Response_time;
}Process;

typedef struct tagNode{
    Process *process;
}Node;

typedef struct tagQueue{
    int capa; // linkedlist 형태의 queue면 딱히 필요없을 듯 ? 
    // nodes 개수 세는 걸로 쓴다 그건 이미 capacitor 값 받아서 노드 수 메모리 할당했음 
    int front;
    int rear;
    int size;
    Node* Nodes;
}Queue;

//함수 이거 따로 headerfile 만드는게 나을 거 같긴한데 일단 복잡해질 거 같아서 여기다 refer 처리 
Process* Create_Process( int pid, int IO );
void CreateQUEUE(Queue** queue, int capacity);
void EnQUEUE(Queue* queue, Process* process);
void Queue_Print(Queue* queue);
void SET_Q(int numofprocess);
void SET_JQ(int numofprocess, int numofIOprocess);


//quicksort 위한 함수들
//arrival로 정렬
int Partition(Node* nodes, int low, int high) { // arrival 기준으로 정렬하고, 같은 arrival에 대해서는 pid 작은 것부터
    // int pivot = nodes[high].process->Arrival_time;
    // int i = (low - 1); // 작은 원소의 인덱스

    // for (int j = low; j < high; j++) {
    //     // 현재 원소가 피벗보다 작으면 교환
    //     if (nodes[j].process->Arrival_time < pivot) {
    //         i++;
    //         Node temp = nodes[i];
    //         nodes[i] = nodes[j];
    //         nodes[j] = temp;
    //     }
    // }
    // // 피벗을 올바른 위치로 교환
    // Node temp = nodes[i + 1];
    // nodes[i + 1] = nodes[high];
    // nodes[high] = temp;
    // return i + 1;
    int pivot = nodes[high].process->Arrival_time;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (nodes[j].process->Arrival_time < pivot ||
            (nodes[j].process->Arrival_time == pivot && nodes[j].process->pid < nodes[high].process->pid)) {
            i++;
            Node temp = nodes[i];
            nodes[i] = nodes[j];
            nodes[j] = temp;
        }
    }
    Node temp = nodes[i + 1];
    nodes[i + 1] = nodes[high];
    nodes[high] = temp;
    return i + 1;
}


void QuickSort(Node* nodes, int low, int high) {
    if (low < high) {
        int pi = Partition(nodes, low, high);

        QuickSort(nodes, low, pi - 1);
        QuickSort(nodes, pi + 1, high);
    }
}
//priority로 정렬
int PriorityPartition(Node* nodes, int low, int high) {
    int pivot = nodes[high].process->Priority;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (nodes[j].process->Priority < pivot) {
            i++;
            Node temp = nodes[i];
            nodes[i] = nodes[j];
            nodes[j] = temp;
        }
    }
    Node temp = nodes[i + 1];
    nodes[i + 1] = nodes[high];
    nodes[high] = temp;
    return i + 1;
}

void PriorityQuickSort(Node* nodes, int low, int high) {
    if (low < high) {
        int pi = PriorityPartition(nodes, low, high);

        PriorityQuickSort(nodes, low, pi - 1);
        PriorityQuickSort(nodes, pi + 1, high);
    }
}

//CPUburst로 정렬 
int CPUburstPartition(Node* nodes, int low, int high) {
    int pivot = nodes[high].process->CPU_burst_time;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (nodes[j].process->CPU_burst_time < pivot) {
            i++;
            Node temp = nodes[i];
            nodes[i] = nodes[j];
            nodes[j] = temp;
        }
    }
    Node temp = nodes[i + 1];
    nodes[i + 1] = nodes[high];
    nodes[high] = temp;
    return i + 1;
}

void CPUburstQuickSort(Node* nodes, int low, int high) {
    if (low < high) {
        int pi = CPUburstPartition(nodes, low, high);

        CPUburstQuickSort(nodes, low, pi - 1);
        CPUburstQuickSort(nodes, pi + 1, high);
    }
}

//글로벌 var 선언해두기 
int CPU_START = 0; 
int CPU_END = 0; 
int CPU_IDLE = 0;
Process* RunningProcess = NULL;

Queue* JOB_Q;
Queue* READY_Q;

Queue* WAIT_Q;
Queue* TERMINATE_Q;
Queue* EVAL_Q;

typedef int ElementType;

void CreateQUEUE( Queue** queue, int capacity){
    (*queue) = (Queue *)malloc(sizeof(Queue));
    (*queue)->Nodes = (Node*)malloc(sizeof(Node)*(capacity)); // 노드 개수는 capacity만큼만
    (*queue)->capa = capacity;
    (*queue)->front = 0;
    (*queue)->rear = -1;
    (*queue)->size = 0 ;
}

void EnQUEUE( Queue* queue, Process *process){
    // queue->Nodes[queue->rear].process=process; 
    // // printf("pid : %d, cpuburst: %d, ioburst: %d\n", queue->Nodes[queue->rear].process->pid,
    // // queue->Nodes[queue->rear].process->CPU_burst_time, queue->Nodes[queue->rear].process->IO_burst_time);
    // queue->rear++;

    if (queue->size == queue->capa) {
        // 큐가 꽉 찼으면 가장 오래된 데이터를 덮어씀
        queue->front = (queue->front + 1) % queue->capa;
    } else {
        queue->size++;
    }
    queue->rear = (queue->rear + 1) % queue->capa;
    queue->Nodes[queue->rear].process = process;
}

Process* DeQUEUE(Queue* queue) {
    if (queue->size == 0) {
        printf("Queue is empty.\n");
        return NULL;
    }
    Process* process = queue->Nodes[queue->front].process;
    queue->front = (queue->front + 1) % queue->capa;
    queue->size--;
    return process;
}

void Queue_Print(Queue* queue){ //print 함수에는 문제 없음 그냥 값 넣는게 문제 있어
    int i = queue->front;
    int count = queue->size;
    while (count > 0) {
        printf("pid : %d, cpuburst: %d, ioburst: %d, arrival time: %d, priority: %d\n",
               queue->Nodes[i].process->pid,
               queue->Nodes[i].process->CPU_burst_time,
               queue->Nodes[i].process->IO_burst_time,
               queue->Nodes[i].process->Arrival_time,
               queue->Nodes[i].process->Priority);
        i = (i + 1) % queue->capa;
        count--;
    }
    // for(int i  = 0; i < queue->rear ; i++){
    //     printf("pid : %d, cpuburst: %d, ioburst: %d, arrival time: %d, priority: %d\n", queue->Nodes[i].process->pid,
    //     queue->Nodes[i].process->CPU_burst_time, queue->Nodes[i].process->IO_burst_time,
    //     queue->Nodes[i].process->Arrival_time, queue->Nodes[i].process->Priority);
    // }
    // printf("pid : %d, cpuburst: %d, ioburst: %d\n", queue->Nodes[6].process->pid,
    // queue->Nodes[6].process->CPU_burst_time, queue->Nodes[6].process->IO_burst_time);
    
}

void Process_Print(Process* process){
    printf("pid : %d, cpuburst: %d, ioburst: %d, arrival time: %d, priority: %d\n",
                process->pid,
               process->CPU_burst_time,
               process->IO_burst_time,
               process->Arrival_time,
               process->Priority);
}

void SET_Q(int numofprocess){
    CreateQUEUE(&JOB_Q, numofprocess);
    CreateQUEUE(&READY_Q, numofprocess);
    CreateQUEUE(&WAIT_Q, numofprocess);
    CreateQUEUE(&TERMINATE_Q, numofprocess); 
    CreateQUEUE(&EVAL_Q, numofprocess);

}

void SET_JQ(int numofprocess, int numofIOprocess){
    // int i;
    // Process arrayofprocess[numofprocess];

    // //queue에 process 채워넣기 
    // for( i = 0 ; i < numofprocess ; i++){
    //     if(numofIOprocess>0){
    //     arrayofprocess[i] = *Create_Process(i, 1); //pid는 순서대로 부여
    //         numofIOprocess--;   
    //     }
    //     else arrayofprocess[i] = *Create_Process(i, 0); //pid는 순서대로 부여
        
    //     EnQUEUE(JOB_Q, &arrayofprocess[i]);
    // }
    
    int i;
    // 각 프로세스를 동적으로 할당하여 함수가 반환된 후에도 지속되도록 합니다.
    for (i = 0; i < numofprocess; i++) {
        Process* new_process;
        if (numofIOprocess > 0) {
            new_process = Create_Process(i, 1); // PID는 순서대로 부여
            numofIOprocess--;
        } else {
            new_process = Create_Process(i, 0); // PID는 순서대로 부여
        }
        EnQUEUE(JOB_Q, new_process);}
    
    QuickSort(JOB_Q->Nodes, 0, JOB_Q->size - 1); // JOBQ를 arrival time 순서대로 정렬
}

void ResetQueue(Queue* queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
}

void SET_READYQ_by_ARRIVALTIME(int numofprocess){ //JQ를 arrival time 순으로 정렬 
    //arrival time .. 순으로 어케 정렬하지 일단 JQ를 복사해서 arrival 순서대로 정렬
    ResetQueue(READY_Q);
    for(int i = 0 ; i < numofprocess; i++){
        EnQUEUE(READY_Q, JOB_Q->Nodes[i].process); //일단 내용 복사하고
    }
        QuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);
}
void SET_READYQ_by_PRIORITY(int numofprocess){
    ResetQueue(READY_Q);
    for(int i = 0 ; i < numofprocess; i++){
        EnQUEUE(READY_Q, JOB_Q->Nodes[i].process); //일단 내용 복사하고
    }
        PriorityQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);
}
void SET_READYQ_by_CPUburst(int numofprocess){
    ResetQueue(READY_Q);
    for(int i = 0 ; i < numofprocess; i++){
        EnQUEUE(READY_Q, JOB_Q->Nodes[i].process); //일단 내용 복사하고
    }
        CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);

}

Process* Create_Process( int pid, int IO){ //random data 부여
    Process *new_process = malloc(sizeof(Process));
    if(new_process == NULL){
        printf("Memory alloc Error");
        return NULL;
    }
    //그냥 일단 임의 지정; 랜덤하게 구성 
    int cpuburst = rand()%10 + 1 ;
    int arrivaltime = rand()%10 + 1 ;
    int priority = rand()%5 + 1;
    
    // printf("enter new pid :  ");
    // scanf(" %d", &(new_process->pid)); //& 안 써도 되나?  그냥 index로 순서대로 pid 생성하는게 simple
    new_process->pid = pid;
    new_process->CPU_burst_time = cpuburst;
    new_process->Arrival_time = arrivaltime;
    new_process->Priority = priority;

    new_process->CPU_burst_remain = cpuburst;
    int ioburst = 0;
    if(IO){ //IO 있으면 IO 관련 변수 set 
        ioburst = rand()%5 + 1;
        new_process->IO_burst_remain = ioburst;   
        new_process->IO_burst_time = ioburst;
    }
    else {
        new_process->IO_burst_time = ioburst;
        new_process->IO_burst_remain = 0;
    }

    int Waiting_time = 0;
    int TurnAround_time = 0;
    int Response_time = 0;

    if (arrivaltime > MAXTIME || arrivaltime < 0) {
        printf("<ERROR> arrivalTime should be in [0..MAX_TIME_UNIT]\n");
        printf("<USAGE> createProcess(int pid, int priority, int arrivalTime, int CPUburst, int IOburst)\n");
        return NULL;
    }
    if (cpuburst <= 0 || ioburst < 0) {
        printf("<ERROR> CPUburst and should be larger than 0 and IOburst cannot be a negative number.\n");
        printf("<USAGE> createProcess(int pid, int priority, int arrivalTime, int CPUburst, int IOburst)\n");
        return NULL;
    }   

    // printf("new process pid: %d\n", new_process->pid); process 생성 문제 없음 
    return new_process; // newprocess의 주소값을 반환해야 하는 거 아니오? 
}

//sort(QuickSort)

int compare(ElementType *a, ElementType *b){
    return(*a - *b);
}
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
//linkedlist_queue 순서 바꾸는 알고리즘 

int giveRandomIO(Process* process ,int cpu_remain_time){
    //running 상태인 프로세스한테 
    int IOwillbehappendafter = rand()%cpu_remain_time;
    return IOwillbehappendafter;
}

int isQueueEmpty(Queue* queue) {
    return queue->size == 0;
}

void schedule_FCFS(){//arrival time 기준 sort를 readyqueue
    //readyqueue의 arrival time 대로 sorting 
    if(RunningProcess ==  NULL){
        if (READY_Q->size > 0) {
            RunningProcess = DeQUEUE(READY_Q);
    }   else {
        RunningProcess = NULL;
    }
}
    
}

void schedule_Non_Preemptive_SJF(){ //burst time
    if (READY_Q->size > 0) {
        CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);
        RunningProcess = DeQUEUE(READY_Q);
    } else {
        RunningProcess = NULL;
    }

}

void schedule_Preemptive_SJF(){//어떻게 뺏지...? -> 이건 들어오는 순서 고려해야되는데
    if (READY_Q->size > 0) {
        CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);
        Process* nextProcess = READY_Q->Nodes[READY_Q->front].process;
        if (RunningProcess == NULL || nextProcess->CPU_burst_remain < RunningProcess->CPU_burst_remain) {
            if (RunningProcess != NULL) {
                EnQUEUE(READY_Q, RunningProcess);
            }
            RunningProcess = DeQUEUE(READY_Q);
        }
    }
}

void schedule_Non_Preemptive_Priority(){
    if (READY_Q->size > 0) {
        PriorityQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);
        RunningProcess = DeQUEUE(READY_Q);
    } else {
        RunningProcess = NULL;
    }
}

void schedule_Preemptive_Priority(){
    if (READY_Q->size > 0) {
        PriorityQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1);
        Process* nextProcess = READY_Q->Nodes[READY_Q->front].process;
        if (RunningProcess == NULL || nextProcess->Priority < RunningProcess->Priority) {
            if (RunningProcess != NULL) {
                EnQUEUE(READY_Q, RunningProcess);
            }
            RunningProcess = DeQUEUE(READY_Q);
        }
    }
}

void schedule_RR(int time_quantum){
    if (RunningProcess == NULL && READY_Q->size > 0) {
        RunningProcess = DeQUEUE(READY_Q);
        CPU_START = 0;
    }

    if (RunningProcess != NULL) {
        CPU_START++;
        if (CPU_START >= time_quantum) {
            EnQUEUE(READY_Q, RunningProcess);
            RunningProcess = NULL;
            CPU_START = 0;
        }
    }
    
}

void simulator(int total_time, int alg, int time_quantum) {
    int t;
    for (t = 0; t < total_time; t++) { // 전체 실행시간이 total_time
        printf("Time: %d\n", t);

        //실행할 거 없으면 종료
        if (isQueueEmpty(JOB_Q) && isQueueEmpty(READY_Q) && isQueueEmpty(WAIT_Q) && RunningProcess==NULL) {
            printf("All queues are empty. Simulation ends at time %d.\n", t);
            break;
        }

        // Job Queue에서 Ready Queue로 프로세스 이동
        while (JOB_Q->size > 0 && JOB_Q->Nodes[JOB_Q->front].process->Arrival_time <= t) {
            EnQUEUE(READY_Q, DeQUEUE(JOB_Q)); //현재보다 arrival time 작거나 같은 것들 다 READY_Q로 옮기기 작업 
            //response time 여기서 해줘야할 거 같은데 
        }
        Queue_Print(READY_Q); 
        
        //running상태 프로세스가 I/O 작업 있다면 
        // if(RunningProcess->IO_burst_time>0){

        // }

    //WAIT_Q에 있던 애들 다시 READY_Q로 불러오기 setup
    int wait_size = WAIT_Q->size; //IO는 병렬적으로 수행된다고 가정
        for (int i = 0; i < wait_size; i++) { //WAIT_Q에 있는 모든 프로세스들에게 적용
            Process* process = DeQUEUE(WAIT_Q);
            process->IO_burst_remain--;
            process->TurnAround_time++;

            if (process->IO_burst_remain > 0) {
                EnQUEUE(WAIT_Q, process); //다시 WAIT_Q로 들어가랏
            } else {
                // process->IO_burst_remain = process->IO_burst_time; //일단 I/O 한 번 하면 없애는 걸로 하는 게 낫지 ..
                //원래의 I/O 시간으로 다시 올려준다.. ** 이부분 어떻게 할 지 생각해야할듯 
                EnQUEUE(READY_Q, process);
            }
        }
    switch (alg) {
            case 0:
                schedule_FCFS();
                break;
            case 1:
                schedule_Non_Preemptive_SJF();
                break;
            case 2:
                schedule_Preemptive_SJF();
                break;
            case 3:
                schedule_Non_Preemptive_Priority();
                break;
            case 4:
                schedule_Preemptive_Priority();
                break;
            case 5:
                schedule_RR(time_quantum);
                break;
        }

        // Running Process 실행 및 상태 갱신
        if (RunningProcess != NULL) {
            // //현재 실행되는 프로세스 출력 
            // printf("This process is running !  ");
            // Process_Print(RunningProcess);

            Queue_Print(WAIT_Q);
            RunningProcess->CPU_burst_remain--;
            RunningProcess->TurnAround_time++;
            for (int i = 0; i < READY_Q->size; i++) {//READY_Q에 있는 애들 기다린 거 표시
                READY_Q->Nodes[(READY_Q->front + i) % READY_Q->capa].process->Waiting_time++;
                READY_Q->Nodes[(READY_Q->front + i) % READY_Q->capa].process->TurnAround_time++;
            }

            if (RunningProcess->CPU_burst_remain == 0) {//끝났으면 TERMINATE_Q
                //만약에 I/O 남아있는 경우면 
                if(RunningProcess->IO_burst_remain > 0){
                    EnQUEUE(WAIT_Q, RunningProcess);
                    RunningProcess = NULL;
                }
                else{
                    printf("Process %d completed at time %d\n", RunningProcess->pid, t + 1);
                }
                //여기까지 출력되고 다음부터 segmentation fault
                EnQUEUE(TERMINATE_Q, RunningProcess);
                RunningProcess = NULL;
            } else if (RunningProcess->IO_burst_remain > 0 & (RunningProcess->CPU_burst_time - RunningProcess->CPU_burst_remain) > 1) {
                //아직 I/O 안끝났으면 다시 WAIT_Q & I/O 있으면 // IOburstremain이 0이상이어야지..
                EnQUEUE(WAIT_Q, RunningProcess);
                RunningProcess = NULL;
            }
        }

        // 시스템이 idle 상태인지 확인
        if (RunningProcess == NULL) {
            CPU_IDLE++;
        }
    
    }

    // 종료된 프로세스들 출력
    printf("\nCompleted Processes:\n");
    Queue_Print(TERMINATE_Q);

}

int main(){
    int numofprocess, numofIOprocess;
    printf("생성할 프로세스의 개수와 IO를 실행할 프로세스 개수를 입력하시오 : ");
    scanf("%d %d", &numofprocess, &numofIOprocess);
    SET_Q(numofprocess);
    SET_JQ(numofprocess, numofIOprocess);

    Queue_Print(JOB_Q);
    printf("\n");

    // SET_READYQ_by_ARRIVALTIME(numofprocess);
    // Queue_Print(READY_Q);
    // printf("\n");

    // SET_READYQ_by_PRIORITY(numofprocess);
    // Queue_Print(READY_Q);

    // ready queue 들어가기 전 상태까진 구현 완료 
    simulator(100, 1, 0);
    return 0;

    // 그냥 바로 큐로 집어넣는게 나은 듯 
    
}