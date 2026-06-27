#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 100
#define Q1_QUANTUM 10
#define Q2_QUANTUM 10
#define Q3_QUANTUM 20
#define BOOST_TIME 50

// Structure to represent a process
typedef struct Process {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_burst_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int first_run_time;
    int queue_level;
    int io_start_time;
    int io_runtime;
    int io_remaining_time;
    int io_completed;
    int executed_time;
    int quantum_used;
    int ready_wait_time;
    int ready_enter_time;
    int io_completion_time;
    int arrived;
    int completed;
    int in_io;
    struct Process* next;
} Process;

// Structure to represent a queue
typedef struct Queue {
    Process* head, * tail;
    int time_quantum;
    int level;
} Queue;

int getQuantumByLevel(int queue_level);
void demoteOrRequeue(Process* p, Queue* q1, Queue* q2, Queue* q3, int current_time);

// Function to create a new process
Process* createProcess(int pid, int arrival, int burst, int io_start, int io_runtime) {
    Process* p = (Process*)malloc(sizeof(Process));

    if (p == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    p->pid = pid;
    p->arrival_time = arrival;
    p->burst_time = burst;
    p->remaining_burst_time = burst;
    p->completion_time = 0;
    p->turnaround_time = 0;
    p->waiting_time = 0;
    p->response_time = -1;
    p->first_run_time = -1;
    p->queue_level = 1; // Start in the highest priority queue
    p->io_start_time = io_start;
    p->io_runtime = io_runtime;
    p->io_remaining_time = 0;
    p->io_completed = (io_runtime <= 0);
    p->executed_time = 0;
    p->quantum_used = 0;
    p->ready_wait_time = 0;
    p->ready_enter_time = 0;
    p->io_completion_time = 0;
    p->arrived = 0;
    p->completed = 0;
    p->in_io = 0;
    p->next = NULL;

    return p;
}

// Function to create a new queue
Queue* createQueue(int quantum, int level) {
    Queue* q = (Queue*)malloc(sizeof(Queue));

    if (q == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    q->head = q->tail = NULL;
    q->time_quantum = quantum;
    q->level = level;

    return q;
}

// Function to add a process to a queue
void enqueue(Queue* q, Process* p) {
    p->next = NULL;

    if (q->tail == NULL) {
        q->head = q->tail = p;
        return;
    }

    q->tail->next = p;
    q->tail = p;
}

void enqueueReady(Queue* q, Process* p, int current_time) {
    p->ready_wait_time = 0;
    p->ready_enter_time = current_time;
    enqueue(q, p);
}

void enqueueReadyResetQuantum(Queue* q, Process* p, int current_time) {
    p->quantum_used = 0;
    enqueueReady(q, p, current_time);
}

// Function to remove a process from the front of a queue
Process* dequeue(Queue* q) {
    Process* p;

    if (q->head == NULL) return NULL;

    p = q->head;
    q->head = q->head->next;
    if (q->head == NULL) q->tail = NULL;
    p->next = NULL;

    return p;
}

int isEmpty(Queue* q) {
    return q->head == NULL;
}

void enqueueByLevel(Queue* q1, Queue* q2, Queue* q3, Process* p, int current_time) {
    if (p->queue_level == 1) {
        enqueueReady(q1, p, current_time);
    }
    else if (p->queue_level == 2) {
        enqueueReady(q2, p, current_time);
    }
    else {
        enqueueReady(q3, p, current_time);
    }
}

void enqueueByLevelResetQuantum(Queue* q1, Queue* q2, Queue* q3, Process* p, int current_time) {
    p->quantum_used = 0;
    enqueueByLevel(q1, q2, q3, p, current_time);
}

int getQuantumByLevel(int queue_level) {
    if (queue_level == 1) return Q1_QUANTUM;
    if (queue_level == 2) return Q2_QUANTUM;
    return Q3_QUANTUM;
}

void checkArrivals(Process* processes[], int n, int current_time, Queue* q1) {
    int i;

    for (i = 0; i < n; i++) {
        if (!processes[i]->arrived && processes[i]->arrival_time == current_time) {
            processes[i]->arrived = 1;
            processes[i]->queue_level = 1;
            processes[i]->ready_wait_time = 0;
            processes[i]->quantum_used = 0;
            enqueueReadyResetQuantum(q1, processes[i], current_time);
        }
    }
}

void checkIoCompletion(Process* processes[], int n, int current_time, Queue* q1, Queue* q2, Queue* q3) {
    int i;

    for (i = 0; i < n; i++) {
        if (processes[i]->in_io && current_time >= processes[i]->io_completion_time) {
            Process* p = processes[i];

            p->in_io = 0;
            p->io_remaining_time = 0;

            enqueueByLevelResetQuantum(q1, q2, q3, p, current_time);
        }
    }
}

void boostQueueToQ1(Queue* from, Queue* q1, int current_time) {
    Process* p;

    while (!isEmpty(from)) {
        p = dequeue(from);
        p->queue_level = 1;
        p->ready_wait_time = 0;
        enqueueReadyResetQuantum(q1, p, current_time);
    }
}

void boostCurrentProcess(Process* p, int current_time) {
    if (p != NULL && p->queue_level > 1) {
        p->queue_level = 1;
        p->ready_wait_time = 0;
        p->ready_enter_time = current_time;
        p->quantum_used = 0;
    }
}

void boostLowerPriorityQueues(Queue* q2, Queue* q3, Queue* q1, Process* current_process, int current_time) {
    if (current_time > 0 && current_time % BOOST_TIME == 0) {
        boostQueueToQ1(q2, q1, current_time);
        boostQueueToQ1(q3, q1, current_time);
        boostCurrentProcess(current_process, current_time);
    }
}

Process* selectNextProcess(Queue* q1, Queue* q2, Queue* q3) {
    if (!isEmpty(q1)) return dequeue(q1);
    if (!isEmpty(q2)) return dequeue(q2);
    if (!isEmpty(q3)) return dequeue(q3);
    return NULL;
}

void demoteOrRequeue(Process* p, Queue* q1, Queue* q2, Queue* q3, int current_time) {
    if (p->queue_level == 1) {
        p->queue_level = 2;
    }
    else if (p->queue_level == 2) {
        p->queue_level = 3;
    }

    enqueueByLevelResetQuantum(q1, q2, q3, p, current_time);
}

void requeueWithoutReset(Process* p, Queue* q1, Queue* q2, Queue* q3, int current_time) {
    enqueueByLevel(q1, q2, q3, p, current_time);
}

int hasHigherPriorityProcess(Process* current_process, Queue* q1, Queue* q2) {
    if (current_process == NULL) return 0;
    if (current_process->queue_level == 2 && !isEmpty(q1)) return 1;
    if (current_process->queue_level == 3 && (!isEmpty(q1) || !isEmpty(q2))) return 1;
    return 0;
}

void preemptIfNeeded(Process** current_process, Queue* q1, Queue* q2, Queue* q3, int current_time) {
    if (hasHigherPriorityProcess(*current_process, q1, q2)) {
        requeueWithoutReset(*current_process, q1, q2, q3, current_time);
        *current_process = NULL;
    }
}

Process* selectRunnableProcess(Queue* q1, Queue* q2, Queue* q3, int current_time) {
    Process* p;

    while (1) {
        p = selectNextProcess(q1, q2, q3);
        if (p == NULL) return NULL;

        if (p->quantum_used >= getQuantumByLevel(p->queue_level)) {
            demoteOrRequeue(p, q1, q2, q3, current_time);
        }
        else {
            return p;
        }
    }
}

// Main MLFQ simulation function
int mlfq_scheduling(Process* processes[], int n) {
    Queue* q1 = createQueue(Q1_QUANTUM, 1);
    Queue* q2 = createQueue(Q2_QUANTUM, 2);
    Queue* q3 = createQueue(Q3_QUANTUM, 3);
    Process* current_process = NULL;
    int current_time = 0;
    int completed_processes = 0;

    while (completed_processes < n) {
        checkArrivals(processes, n, current_time, q1);
        checkIoCompletion(processes, n, current_time, q1, q2, q3);
        boostLowerPriorityQueues(q2, q3, q1, current_process, current_time);
        preemptIfNeeded(&current_process, q1, q2, q3, current_time);

        if (current_process == NULL) {
            current_process = selectRunnableProcess(q1, q2, q3, current_time);

            if (current_process != NULL && current_process->first_run_time == -1) {
                current_process->first_run_time = current_time;
                current_process->response_time = current_time - current_process->arrival_time;
            }
        }

        if (current_process == NULL) {
            current_time++;
            continue;
        }

        if (!current_process->io_completed &&
            current_process->io_runtime > 0 &&
            current_process->executed_time == current_process->io_start_time) {
            current_process->io_completed = 1;
            current_process->in_io = 1;
            current_process->io_remaining_time = current_process->io_runtime;
            current_process->io_completion_time = current_time + current_process->io_runtime;
            current_process = NULL;
            continue;
        }

        current_process->remaining_burst_time--;
        current_process->executed_time++;
        current_process->quantum_used++;
        current_time++;

        if (current_process->remaining_burst_time == 0) {
            current_process->completed = 1;
            current_process->completion_time = current_time;
            current_process->turnaround_time = current_process->completion_time - current_process->arrival_time;
            current_process->waiting_time = current_process->turnaround_time - current_process->burst_time;
            completed_processes++;
            current_process = NULL;
        }
        else if (!current_process->io_completed &&
            current_process->io_runtime > 0 &&
            current_process->executed_time == current_process->io_start_time) {
            current_process->io_completed = 1;
            current_process->in_io = 1;
            current_process->io_remaining_time = current_process->io_runtime;
            current_process->io_completion_time = current_time + current_process->io_runtime;
            current_process = NULL;
        }
        else if (current_process->quantum_used >=
                getQuantumByLevel(current_process->queue_level)) {
            demoteOrRequeue(current_process, q1, q2, q3, current_time);
            current_process = NULL;
        }
    }

    free(q1);
    free(q2);
    free(q3);

    return current_time;
}

int readProcesses(const char* filename, Process* processes[]) {
    FILE* fp;
    int n = 0;
    int pid, arrival_time, run_time, io_start_time, io_runtime;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open input file: %s\n", filename);
        return -1;
    }

    while (n < MAX_PROCESSES &&
        fscanf(fp, "%d %d %d %d %d", &pid, &arrival_time, &run_time, &io_start_time, &io_runtime) == 5) {
        processes[n++] = createProcess(pid, arrival_time, run_time, io_start_time, io_runtime);
    }

    fclose(fp);
    return n;
}

int main(int argc, char* argv[]) {
    Process* processes[MAX_PROCESSES];
    const char* input_file = "trace1.txt";
    int n;
    int final_time;
    int i;

    if (argc >= 2) {
        input_file = argv[1];
    }

    n = readProcesses(input_file, processes);
    if (n <= 0) {
        if (n == 0) {
            printf("No process data found.\n");
        }
        return 1;
    }

    final_time = mlfq_scheduling(processes, n);

    printf("PID\tTurnaround Time\tResponse Time\n");
    for (i = 0; i < n; i++) {
        printf("%d\t%d\t\t%d\n",
            processes[i]->pid,
            processes[i]->turnaround_time,
            processes[i]->response_time);
    }
    printf("Final time: %d\n", final_time);

    for (i = 0; i < n; i++) {
        free(processes[i]);
    }

    return 0;
}
