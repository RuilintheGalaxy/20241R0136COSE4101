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

    int pivot_priority = nodes[high].process->Priority;
    int pivot_burst_remain = nodes[high].process->CPU_burst_remain;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if ((nodes[j].process->Priority < pivot_priority) ||
            (nodes[j].process->Priority == pivot_priority && nodes[j].process->CPU_burst_remain < pivot_burst_remain) ||
            (nodes[j].process->Priority == pivot_priority && nodes[j].process->CPU_burst_remain == pivot_burst_remain && nodes[j].process->pid < nodes[high].process->pid)) {
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
void sortQueueByPriority(Queue* queue) {
    if (queue->size == 0) return;

    int n = queue->size;
    Node* tempNodes = (Node*)malloc(sizeof(Node) * n);
    int i = queue->front;
    for (int j = 0; j < n; j++) {
        tempNodes[j] = queue->Nodes[i];
        i = (i + 1) % queue->capa;
    }

    PriorityQuickSort(tempNodes, 0, n - 1);

    i = queue->front;
    for (int j = 0; j < n; j++) {
        queue->Nodes[i] = tempNodes[j];
        i = (i + 1) % queue->capa;
    }

    free(tempNodes);
}
//CPUburst로 정렬 
int CPUburstPartition(Node* nodes, int low, int high) {
    int pivot = nodes[high].process->CPU_burst_remain;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (nodes[j].process->CPU_burst_remain < pivot ||
            (nodes[j].process->CPU_burst_remain == pivot && nodes[j].process->pid < nodes[high].process->pid)) {
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
void sortQueueByCPUburst(Queue* queue) {
    if (queue->size == 0) return;

    int n = queue->size;
    Node* tempNodes = (Node*)malloc(sizeof(Node) * n);
    int i = queue->front;
    for (int j = 0; j < n; j++) {
        tempNodes[j] = queue->Nodes[i];
        i = (i + 1) % queue->capa;
    }

    CPUburstQuickSort(tempNodes, 0, n - 1);

    i = queue->front;
    for (int j = 0; j < n; j++) {
        queue->Nodes[i] = tempNodes[j];
        i = (i + 1) % queue->capa;
    }

    free(tempNodes);
}

int IORemainPartition(Node* nodes, int low, int high) {
    int pivot = nodes[high].process->IO_burst_remain;
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (nodes[j].process->IO_burst_remain > pivot ||
            (nodes[j].process->IO_burst_remain == pivot && nodes[j].process->pid < nodes[high].process->pid)) {
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
void IORemainQuickSort(Node* nodes, int low, int high) {
    if (low < high) {
        int pi = IORemainPartition(nodes, low, high);

        IORemainQuickSort(nodes, low, pi - 1);
        IORemainQuickSort(nodes, pi + 1, high);
    }
}
void sortQueueByIORemain(Queue* queue) {
    if (queue->size == 0) return;

    int n = queue->size;
    Node* tempNodes = (Node*)malloc(sizeof(Node) * n);
    int i = queue->front;
    for (int j = 0; j < n; j++) {
        tempNodes[j] = queue->Nodes[i];
        i = (i + 1) % queue->capa;
    }

    IORemainQuickSort(tempNodes, 0, n - 1);

    i = queue->front;
    for (int j = 0; j < n; j++) {
        queue->Nodes[i] = tempNodes[j];
        i = (i + 1) % queue->capa;
    }

    free(tempNodes);
}
//글로벌 var 선언해두기 
int CPU_START = 0; 
int CPU_END = 0; 
int CPU_IDLE = 0;
Process* RunningProcess = NULL;
Queue* JOB_Q;
Queue* BACKUP_JOB_Q;
Queue* READY_Q;
Queue* WAIT_Q;
Queue* TERMINATE_Q;
Queue* EVAL_Q;

void CreateQUEUE( Queue** queue, int capacity){
    (*queue) = (Queue *)malloc(sizeof(Queue));
    (*queue)->Nodes = (Node*)malloc(sizeof(Node)*(capacity)); // 노드 개수는 capacity만큼만
    (*queue)->capa = capacity;
    (*queue)->front = 0;
    (*queue)->rear = -1;
    (*queue)->size = 0 ;
}

void EnQUEUE( Queue* queue, Process *process){

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

void ResetQueue(Queue* queue) {
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
}

int isQueueEmpty(Queue* queue) {
    return queue->size == 0;
}

void Queue_Print(Queue* queue){ //print 함수에는 문제 없음 그냥 값 넣는게 문제 있어
    int i = queue->front;
    int count = queue->size;
    while (count > 0) {

        printf("pid : %d, cpuburst: %d, ioburst: %d, arrival time: %d, priority: %d, io_occurring time: %d, cpuburst_remain: %d, ioburst_remain: %d, TimeInRunning: %d, Waiting_time: %d, TurnAround_time: %d\n",
               queue->Nodes[i].process->pid,
               queue->Nodes[i].process->CPU_burst_time,
               queue->Nodes[i].process->IO_burst_time,
               queue->Nodes[i].process->Arrival_time,
               queue->Nodes[i].process->Priority,
               queue->Nodes[i].process->IO_occuring_time,
               queue->Nodes[i].process->CPU_burst_remain,
               queue->Nodes[i].process->IO_burst_remain,
               queue->Nodes[i].process->TimeInRunning,
               queue->Nodes[i].process->Waiting_time,
               queue->Nodes[i].process->TurnAround_time);

        i = (i + 1) % queue->capa;
        count--;
    }
    
}

void Process_Print(Process* process){
   
    printf("pid : %d, cpuburst: %d, ioburst: %d, arrival time: %d, priority: %d, io_occurring time: %d, cpuburst_remain: %d, ioburst_remain: %d, TimeInRunning: %d, Waiting_time: %d, TurnAround_time: %d\n",
               process->pid,
               process->CPU_burst_time,
               process->IO_burst_time,
               process->Arrival_time,
               process->Priority,
               process->IO_occuring_time,
               process->CPU_burst_remain,
               process->IO_burst_remain,
               process->TimeInRunning,
               process->Waiting_time,
               process->TurnAround_time);
}

void SET_Q(int numofprocess){
    CreateQUEUE(&JOB_Q, numofprocess);
    CreateQUEUE(&READY_Q, numofprocess);
    CreateQUEUE(&WAIT_Q, numofprocess);
    CreateQUEUE(&TERMINATE_Q, numofprocess); 
    CreateQUEUE(&EVAL_Q, numofprocess);
    CreateQUEUE(&BACKUP_JOB_Q, numofprocess);
}

void SET_JQ(int numofprocess, int numofIOprocess){
    
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

Process* Create_Process(int pid, int IO){ //random data 부여
    Process *new_process = malloc(sizeof(Process));
    if(new_process == NULL){
        printf("Memory alloc Error");
        return NULL;
    }
    //그냥 일단 임의 지정; 랜덤하게 구성 
    int cpuburst = rand()%7 + 1 ;
    int arrivaltime = rand()%10 + 1 ;
    int priority = rand()%5 + 1; // 1부터 5까지 .. 
    
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
        new_process->IO_occuring_time = rand()%(new_process->CPU_burst_remain) + 1;
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


//스케줄링 알고리즘들
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

void schedule_Non_Preemptive_SJF(){ //ok burst time
    if(RunningProcess == NULL){
        if (READY_Q->size > 0) {
            // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear); //*****  정렬은 front, rear로 해야함
            sortQueueByCPUburst(READY_Q);
            // printf("알고리즘 후 Sorted READY_Q by CPU_burst_remain:\n");
            // Queue_Print(READY_Q);

            RunningProcess = DeQUEUE(READY_Q);

            printf("Scheduled Process: %d\n", RunningProcess->pid);
        } else {

            // CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1); //*****  
            RunningProcess = NULL;
        }
    }
}

void schedule_Preemptive_SJF(){//어떻게 뺏지...? -> 이건 들어오는 순서 고려해야되는데
    if(RunningProcess == NULL){
        if (READY_Q->size > 0) {
            // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear); //*****  정렬은 front, rear로 해야함

            sortQueueByCPUburst(READY_Q);

            // printf("알고리즘 후 Sorted READY_Q by CPU_burst_remain:\n");
            // Queue_Print(READY_Q);

            RunningProcess = DeQUEUE(READY_Q);
            // printf("Scheduled Process: %d\n", RunningProcess->pid);
        } else {
            // CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1); //*****  
            RunningProcess = NULL;
        }
    }
    else{//실행중인 running process가 있는 경우 
        if(READY_Q->size >0){
            sortQueueByCPUburst(READY_Q);

            // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear);
            Process* nextProcess = READY_Q->Nodes[READY_Q->front].process;  
            if (RunningProcess == NULL || (nextProcess->CPU_burst_remain) < RunningProcess->CPU_burst_remain) {
                EnQUEUE(READY_Q, RunningProcess);
                sortQueueByCPUburst(READY_Q);

                // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear);

                RunningProcess = DeQUEUE(READY_Q);  
            }
        }
    }
    

}

void schedule_Non_Preemptive_Priority(){ //ok
    if(RunningProcess == NULL){
        if (READY_Q->size > 0) {
            // PriorityQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear);
            sortQueueByPriority(READY_Q);
            RunningProcess = DeQUEUE(READY_Q);
            // printf("Scheduled Process: %d\n", RunningProcess->pid);

        } else {
            RunningProcess = NULL;
        }

    }
}

void schedule_Preemptive_Priority(){
    if(RunningProcess == NULL){
        if (READY_Q->size > 0) {
            // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear); //*****  정렬은 front, rear로 해야함

            sortQueueByPriority(READY_Q);

            // printf("알고리즘 후 Sorted READY_Q by priority:\n");
            // Queue_Print(READY_Q);

            RunningProcess = DeQUEUE(READY_Q);
            // printf("Scheduled Process: %d\n", RunningProcess->pid);
        } else {
            // CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1); //*****  
            RunningProcess = NULL;
        }
    }
    else{//실행중인 running process가 있는 경우 
        if(READY_Q->size >0){
            sortQueueByPriority(READY_Q);

            // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear);
            Process* nextProcess = READY_Q->Nodes[READY_Q->front].process;  
            if (RunningProcess == NULL || (nextProcess->Priority) < RunningProcess->Priority) {
                EnQUEUE(READY_Q, RunningProcess);
                sortQueueByPriority(READY_Q);

                // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear);

                RunningProcess = DeQUEUE(READY_Q);  
            }
        }
    }
}

void schedule_RR(int time_quantum){
    if (RunningProcess == NULL && READY_Q->size > 0) {
        RunningProcess = DeQUEUE(READY_Q);
        CPU_START = 0;
    }
    // printf("cpu start 값 : %d\n", CPU_START);

    if (RunningProcess != NULL) {
        CPU_START++;
        if (CPU_START > time_quantum) {
            EnQUEUE(READY_Q, RunningProcess);
            if(READY_Q->size > 0) {
                RunningProcess = DeQUEUE(READY_Q);}
            else RunningProcess = NULL;
            CPU_START = 1;
        }
    }
    
}

void schedule_Non_Preemptive_LongestIOfirst(){ //ok burst time
    if(RunningProcess == NULL){
        if (READY_Q->size > 0) {
            // CPUburstQuickSort(READY_Q->Nodes, READY_Q->front, READY_Q->rear); //*****  정렬은 front, rear로 해야함
            sortQueueByIORemain(READY_Q);
            // printf("알고리즘 후 Sorted READY_Q by CPU_burst_remain:\n");
            // Queue_Print(READY_Q);

            RunningProcess = DeQUEUE(READY_Q);

            printf("Scheduled Process: %d\n", RunningProcess->pid);
        } else {

            // CPUburstQuickSort(READY_Q->Nodes, 0, READY_Q->size - 1); //*****  
            RunningProcess = NULL;
        }
    }
}
// 시뮬레이터에서 각 시간 단위의 실행 상태를 기록하기 위한 구조체
typedef struct tagGanttChart {
    int time;
    int pid;
} GanttChart;

GanttChart gantt[100];
int ganttSize = 0;

void resetGanttChart() {
    ganttSize = 0;
    for (int i = 0; i < 50; i++) {
        gantt[i].time = -1;  // 초기화 값으로 -1을 사용합니다.
        gantt[i].pid = -1;   // 초기화 값으로 -1을 사용합니다.
    }
}
void printGanttChart() {
    printf("\nGantt Chart:\n");
    for(int i = 0; i < ganttSize+1; i++){
        printf("|  %d   ", i);

    }
    printf("|\n");
    printf("chart size is .. %d\n", ganttSize);
    for (int i = 0; i < ganttSize; i++) {
        if (gantt[i].pid == -1) {
            printf("| idle " );
        } else {
            printf("| P%d   ", gantt[i].pid);
        }
    }
    printf("|\n");
}



void simulator(int total_time, int alg, int time_quantum) {
    CPU_IDLE = 0; 
    int t;
    for (t = 0; t < total_time; t++) { // 전체 실행시간이 total_time
        //***** printf("\nTime: %d\n", t);

        //실행할 거 없으면 종료
        if (isQueueEmpty(JOB_Q) && isQueueEmpty(READY_Q) && isQueueEmpty(WAIT_Q) && RunningProcess==NULL) {
            printf("All queues are empty. Simulation ends at time %d.\n", t);
            // printf("cpu utilization : %d / %d ", CPU_IDLE/t);
            break;
        }

        // 시간(arrival time)이 되었으면, Job Queue에서 Ready Queue로 프로세스 이동
        while (JOB_Q->size > 0 && JOB_Q->Nodes[JOB_Q->front].process->Arrival_time <= t) {
            EnQUEUE(READY_Q, DeQUEUE(JOB_Q)); //현재보다 arrival time 작거나 같은 것들 다 READY_Q로 옮기기 작업 
        }
        // printf("%d",isQueueEmpty(READY_Q));// readyqueue가 비워진대.. 흠

        //알고리즘에 따라 정렬하기 전의 readyQueue
        //*****Queue_Print(READY_Q); 
        // printf("\n ");
    // Process* comebackprocess = NULL;
    //WAIT_Q에 있던 애들 다시 READY_Q로 불러오기 setup
        int wait_size = WAIT_Q->size; //IO는 병렬적으로 수행된다고 가정
        for (int i = 0; i < wait_size; i++) { //WAIT_Q에 있는 모든 프로세스들에게 적용
            Process* process = DeQUEUE(WAIT_Q);
            process->IO_burst_remain--;
            process->TurnAround_time++;

            // printf("process who is waiting : %d, io_burst_remain : %d ",
            // process->pid,
            // process->IO_burst_remain);

            // printf("waiting : ");
            // Process_Print(process);

            if (process->IO_burst_remain > 0) {
                EnQUEUE(WAIT_Q, process); //다시 WAIT_Q로 들어가랏
            }
            else {
                // process->IO_burst_remain = process->IO_burst_time; //일단 I/O 한 번 하면 없애는 걸로 하는 게 낫지 ..
                //원래의 I/O 시간으로 다시 올려준다.. ** 이부분 어떻게 할 지 생각해야할듯 
                // if(process->CPU_burst_remain == 0 ){
                //     EnQUEUE(TERMINATE_Q, process);
                // }
                // else 
                //멍청이코드****** 아님 뒤에서 다시 올려주기 때문에 한 번 줄여놔야함 기다리지도 않았는데 루프 안에 같이 있어서 waiting time 늘어나거덩
                //다시 readyqueue로 복귀 
                if(process->CPU_burst_remain>0) process->Waiting_time--; //지금 들어가면 기다린 적 없는 거임
                process->TurnAround_time--;
                // comebackprocess = process;
                EnQUEUE(READY_Q, process); //얘가 readyQUEUE로 제대로 안 들어가는 듯 
                
                // printf("come back to ready queue~!\n");
                // Queue_Print(READY_Q);
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
            case 6:
                schedule_Non_Preemptive_LongestIOfirst();
                break;
        }
        //***** printf("\n");
        // Queue_Print(READY_Q);
        // Running Process 실행 및 상태 갱신
        if (RunningProcess != NULL) {
            // //현재 실행되는 프로세스 출력 
            // if(comebackprocess != NULL){
            //     EnQUEUE(READY_Q, comebackprocess);
            // }
            
            printf("This process is running !  ");
            Process_Print(RunningProcess);

            gantt[ganttSize].time = t;
            gantt[ganttSize].pid = RunningProcess->pid;
            ganttSize++;

            // Queue_Print(WAIT_Q);
            RunningProcess->CPU_burst_remain--;
            RunningProcess->TurnAround_time++;
            
            if (RunningProcess->CPU_burst_remain < 0) {
                RunningProcess->CPU_burst_remain = 0; // 음수가 되지 않도록 방지 왜냐면 이미 IO 끝나고 들어왔을 때, cpu burst 안 남아있는 경우 있음 
            }

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
                    // printf("Process %d completed at time %d\n", RunningProcess->pid, t + 1);
                    EnQUEUE(TERMINATE_Q, RunningProcess);
                    RunningProcess = NULL;
                }
            } 
            else if (RunningProcess->IO_burst_remain > 0 && (RunningProcess->IO_occuring_time == RunningProcess->CPU_burst_remain)) {
                //아직 I/O 안끝났으면 다시 WAIT_Q & I/O 있으면 // IOburstremain이 0이상이어야지..
                //IO 발생타임 = > CPUburst 남은 시간이랑 특정 랜덤 I/O 발생시간이랑 같으면 
                EnQUEUE(WAIT_Q, RunningProcess);
                RunningProcess = NULL;
            }
        }
        else {
            // CPU가 idle인 경우 기록
            gantt[ganttSize].time = t;
            gantt[ganttSize].pid = -1; // idle 상태를 나타내기 위해 -1 사용
            ganttSize++;
        }
        // 시스템이 idle 상태인지 확인 : 딱히 필요없으심~
        if (RunningProcess == NULL) {
            CPU_IDLE++;
        }
    }

    // 종료된 프로세스들 출력
    printf("\nCompleted Processes:\n");
    Queue_Print(TERMINATE_Q);
    // printf("CPU IDLE accum : %d", CPU_IDLE);
    //cpu utilization ;;;

}

void CopyQueue(Queue* source, Queue* destination) { //이건 포인터 복사임 ..
    // destination 큐를 초기화합니다.
    ResetQueue(destination);
    
    // source 큐의 각 프로세스를 destination 큐로 복사합니다.
    int i = source->front;
    int count = source->size;
    while (count > 0) {
        Process* process = source->Nodes[i].process;
        EnQUEUE(destination, process);
        i = (i + 1) % source->capa;
        count--;
    }
}

Process* DeepCopyProcess(Process* original) {
    if (original == NULL) {
        return NULL;
    }
    
    Process* copy = (Process*)malloc(sizeof(Process));
    if (copy == NULL) {
        printf("Memory allocation error\n");
        return NULL;
    }

    // 모든 필드 복사
    copy->pid = original->pid;
    copy->CPU_burst_time = original->CPU_burst_time;
    copy->IO_burst_time = original->IO_burst_time;
    copy->Arrival_time = original->Arrival_time;
    copy->Priority = original->Priority;

    copy->CPU_burst_remain = original->CPU_burst_remain;
    copy->IO_burst_remain = original->IO_burst_remain;
    copy->IO_occuring_time = original->IO_occuring_time;
    copy->TimeInRunning = original->TimeInRunning;

    copy->Waiting_time = original->Waiting_time;
    copy->TurnAround_time = original->TurnAround_time;
    copy->Response_time = original->Response_time;

    return copy;
}
void DeepCopyQueue(Queue* original, Queue** copy) {
    if (original == NULL) {
        *copy = NULL;
        return;
    }

    // 새로운 큐 생성
    CreateQUEUE(copy, original->capa);
    (*copy)->size = original->size;
    (*copy)->front = original->front;
    (*copy)->rear = original->rear;

    // 각 노드를 깊은 복사
    for (int i = 0; i < original->capa; i++) {
        if (original->Nodes[i].process != NULL) {
            (*copy)->Nodes[i].process = DeepCopyProcess(original->Nodes[i].process);
        } else {
            (*copy)->Nodes[i].process = NULL;
        }
    }
}

typedef struct tagMetrics {
    double avg_turnaround_time;
    double avg_waiting_time;
} Metrics;

Metrics metrics[7];

void calculateMetrics(Queue* terminate_queue, int alg) {
    int total_turnaround_time = 0;
    int total_waiting_time = 0;
    int num_processes = terminate_queue->size;

    for (int i = 0; i < num_processes; i++) {
        Process* process = terminate_queue->Nodes[(terminate_queue->front + i) % terminate_queue->capa].process;
        total_turnaround_time += process->TurnAround_time;
        total_waiting_time += process->Waiting_time;
    }

    metrics[alg].avg_turnaround_time = (double)total_turnaround_time / num_processes;
    metrics[alg].avg_waiting_time = (double)total_waiting_time / num_processes;
}

void printMetrics() {
    const char* alg_names[] = {
        "FCFS",
        "Non-Preemptive SJF",
        "Preemptive SJF",
        "Non-Preemptive Priority",
        "Preemptive Priority",
        "Round Robin",
        "Longest IO First"
    };

    printf("\nAlgorithm Metrics:\n");
    for (int i = 0; i < 7; i++) {
        printf("%s - Avg Turnaround Time: %.2f, Avg Waiting Time: %.2f\n",
               alg_names[i],
               metrics[i].avg_turnaround_time,
               metrics[i].avg_waiting_time);
    }
}


int main(){
    int numofprocess, numofIOprocess;
    printf("생성할 프로세스의 개수와 IO를 실행할 프로세스 개수를 입력하시오 : ");
    scanf("%d %d", &numofprocess, &numofIOprocess);
    SET_Q(numofprocess);
    SET_JQ(numofprocess, numofIOprocess);
    // Make_Priority_Q(numofprocess);
    
    Queue_Print(JOB_Q);
    printf("\n");

    DeepCopyQueue(JOB_Q, &BACKUP_JOB_Q); // backup 잘 됨 확인 . 
 
   /*
    CopyQueue(BACKUP_JOB_Q, JOB_Q);
   */ 
    //simulator 0 : FCFS
    int test ; 
    int chosen_algo ; 
    int timequantum ;
    for(test = 0 ; test < 7 ; test++ ){
        printf("tell me what you want to see : \n 0 : FCFS , 1 : Non-Preemptive SJF, 2 : Preemptive SJF, \n 3 : Non-Preemptive Priority, 4 : Preemptive Priority, 5 : RoundRobin, 6 : Longest_IO_First");
        scanf("%d", &chosen_algo);
        if(chosen_algo == 5){
            printf("tell me what time quantum do you want : ");
            scanf("%d", &timequantum);
        }
        DeepCopyQueue(BACKUP_JOB_Q, &JOB_Q);
        resetGanttChart();
        simulator(MAXTIME, chosen_algo, timequantum);
        printf("\n");
        printf("simulator you selected was %d \n", chosen_algo);
        printGanttChart();
        printf("\n");
        calculateMetrics(TERMINATE_Q, chosen_algo);
        ResetQueue(TERMINATE_Q);
    }

    printMetrics();
    
    return 0;

}


