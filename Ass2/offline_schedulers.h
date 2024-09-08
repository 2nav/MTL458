#pragma once

// Can include any other headers as needed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>

#include <stdint.h>
#include <sys/mman.h>

int INSIZE = 1024;
static bool *GLOB_ERROR;

#define MAX_COMMANDS 50

typedef struct
{

    // This will be given by the tester function this is the process command to be scheduled
    char *command;

    // Temporary parameters for your usage can modify them as you wish
    bool finished; // If the process is finished safely
    bool error;    // If an error occurs during execution
    uint64_t start_time;
    uint64_t completion_time;
    uint64_t turnaround_time;
    uint64_t waiting_time;
    uint64_t response_time;
    bool started;
    int process_id;
    uint64_t burst_time;
    uint8_t priority;

} Process;

/**
  ____  _    _ ______ _    _ ______
 / __ \| |  | |  ____| |  | |  ____|
| |  | | |  | | |__  | |  | | |__
| |  | | |  | |  __| | |  | |  __|
| |__| | |__| | |____| |__| | |____
 \___\_\\____/|______|\____/|______|
 */

// Queue implementation
typedef struct ProcessQueue
{
    Process *processes[MAX_COMMANDS];
    int head;
    int tail;
    int size;
} ProcessQueue;

// Function to create a queue
ProcessQueue *createQueue()
{
    ProcessQueue *q = (ProcessQueue *)malloc(sizeof(ProcessQueue));
    q->head = -1;
    q->tail = -1;
    return q;
}

// Function to add an element to the queue
void enqueue(ProcessQueue *q, Process *p)
{
    if (q->head == -1)
    {
        q->head = 0;
    }
    q->tail = (q->tail + 1);
    q->processes[q->tail] = p;
}

// Function to remove an element from the queue
Process *dequeue(ProcessQueue *q)
{
    if (q->head == -1) // Check if the queue is empty
    {
        return NULL; // Return NULL if the queue is empty
    }

    Process *p = q->processes[q->head]; // Get a pointer to the process

    if (q->head == q->tail)
    {
        q->head = -1;
        q->tail = -1;
    }
    else
    {
        q->head = q->head + 1;
    }

    return p; // Return the pointer to the process
}

// Function to check if the queue is empty
bool isEmpty(ProcessQueue *q)
{
    return q->head == -1;
}

// Function to get the size of the queue
int size(ProcessQueue *q)
{
    return isEmpty(q) ? 0 : q->tail - q->head + 1;
}

// Function prototypes
void FCFS(Process p[], int n);
void RoundRobin(Process p[], int n, int quantum);
void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime);

/**
 _    _      _
| |  | |    | |
| |__| | ___| |_ __   ___ _ __ ___
|  __  |/ _ \ | '_ \ / _ \ '__/ __|
| |  | |  __/ | |_) |  __/ |  \__ \
|_|  |_|\___|_| .__/ \___|_|  |___/
              | |
              |_|
 */
void inputParser(char *str, char *args[INSIZE])
{
    char *token = strtok(str, " \t");
    int i = 0;
    while (token != NULL)
    {
        args[i] = token;
        token = strtok(NULL, " \t");
        i++;
    }
    args[i] = '\0';
}

/**
 * @brief Get time in milliseconds
 *
 * @return uint64_t
 */
uint64_t get_time()
{
    // stackoverflow shit
    struct timeval timebuff;
    gettimeofday(&timebuff, NULL);
    return timebuff.tv_sec * 1000 + timebuff.tv_usec / 1000;
}

/**
 * @brief Get time in microseconds
 *
 * @return uint64_t
 */
uint64_t get_time_micro()
{
    struct timeval timebuff;
    gettimeofday(&timebuff, NULL);
    return timebuff.tv_sec * 1000000 + timebuff.tv_usec;
}

/* ______ _____ ______ _____
|  ____/ ____|  ____/ ____|
| |__ | |    | |__ | (___
|  __|| |    |  __| \___ \
| |   | |____| |    ____) |
|_|    \_____|_|   |_____/
*/

/**
 * @brief FCFS Scheduler
 *
 * @param p - the process array
 * @param n - number of processes
 */
void FCFS(Process p[], int n)
{

    int i = 0;
    uint64_t start = get_time();
    // printf("Start time: %f\n", start);
    while (i < n)
    {

        // https://stackoverflow.com/a/13274800
        GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *GLOB_ERROR = false;

        // Execute the process
        p[i].finished = false;
        p[i].error = false;

        // parse tokens
        char *args[INSIZE];
        int commandSize = strlen(p[i].command);
        char command[commandSize + 1];
        strcpy(command, p[i].command);
        command[commandSize] = '\0';
        inputParser(command, args);

        p[i].start_time = get_time();

        p[i].process_id = fork();
        if (p[i].process_id == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                // printf("Error executing command\n");
                *GLOB_ERROR = true;
                exit(1);
            }
            exit(0);
        }
        // Wait for the process to finish
        else
        {
            // waitpid(p[i].process_id, NULL, 0);
            wait(NULL);
            if (*GLOB_ERROR)
            {
                p[i].error = true;
            }

            // Update the process parameters
            p[i].finished = true;
            time_t end = get_time();
            p[i].completion_time = end;
            p[i].turnaround_time = p[i].completion_time - start;
            p[i].waiting_time = p[i].turnaround_time - (p[i].completion_time - p[i].start_time); // FCFS has no waiting time, TAT = BT
            p[i].response_time = p[i].start_time - start;

            printf("%s|%ld|%ld\n", p[i].command, p[i].start_time - start, p[i].completion_time - start);
            // printf("Completion time: %ld\n Turnaround time: %ld\n Waiting time: %ld\n Response time: %ld\n", p[i].completion_time, p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
            i++;
        }
    }

    // create a csv and dump Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time
    FILE *f = fopen("result_offline_FCFS.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);
    for (int i = 0; i < n; i++)
    {
        fprintf(f, "%s,%s,%s,%ld,%ld,%ld,%ld\n", p[i].command, p[i].finished ? "Yes" : "No", p[i].error ? "Yes" : "No", p[i].completion_time - p[i].start_time, p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
        fflush(f);
    }
    fclose(f);
}

/*
 _____  _____
|  __ \|  __ \
| |__) | |__) |
|  _  /|  _  /
| | \ \| | \ \
|_|  \_\_|  \_\
*/

/**
 * @brief Round Robin Scheduler
 *
 * @param p - the process array
 * @param n - number of processes
 * @param quantum - time quantum
 */
void RoundRobin(Process p[], int n, int quantum)
{
    int i = 0;
    int completed = 0;

    GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *GLOB_ERROR = false;

    uint64_t start = get_time();

    FILE *f = fopen("result_offline_RR.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);

    // initialize process parameters
    for (int i = 0; i < n; i++)
    {
        p[i].finished = false;
        p[i].error = false;
        p[i].started = false;
        p[i].burst_time = 0;
    }

    while (completed < n)
    {
        uint64_t cont_start, cont_end = start;
        i %= n;

        if (p[i].finished || p[i].error)
        {
            i++;
            continue;
        }
        if (p[i].started)
        {
            kill(p[i].process_id, SIGCONT);
        }
        else
        {
            p[i].start_time = get_time();
            p[i].started = true;
            p[i].process_id = fork();
        }

        if (p[i].process_id == 0)
        {
            char *args[INSIZE];
            int commandSize = strlen(p[i].command);
            char command[commandSize + 1];
            strcpy(command, p[i].command);
            command[commandSize] = '\0';
            inputParser(command, args);
            if (execvp(args[0], args) == -1)
            {
                *GLOB_ERROR = true;
                // printf("Error executing command\n");
                exit(1);
            }
            printf("Process %s ended\n", p[i].command);
            exit(0);
        }
        else
        {
            cont_start = get_time() - start;
            usleep(quantum * 1000);
            p[i].burst_time += quantum;
            cont_end = get_time() - start;
            printf("%s|%ld|%ld\n", p[i].command, cont_start, cont_end);
            // cont_start = cont_end;
            int status = waitpid(p[i].process_id, NULL, WNOHANG);
            if (status == 0)
            {
                kill(p[i].process_id, SIGSTOP);
            }
            else
            { // check if finished

                uint64_t end = get_time();

                p[i].finished = true;
                completed++;
                if (*GLOB_ERROR)
                {
                    p[i].error = true;
                    p[i].finished = false;
                    *GLOB_ERROR = false;
                }

                p[i].completion_time = end;
                p[i].turnaround_time = p[i].completion_time - start;
                p[i].waiting_time = p[i].turnaround_time - p[i].burst_time;
                p[i].response_time = p[i].start_time - start;

                fprintf(f, "%s,%s,%s,%ld,%ld,%ld,%ld\n", p[i].command, p[i].finished ? "Yes" : "No", p[i].error ? "Yes" : "No", p[i].burst_time, p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
                fflush(f);
            }

            i++;
        }
    }
    // printf("Completed time %ld\n", get_time() - start);
    fclose(f);
}

/*
 __  __ _      ______ ____
|  \/  | |    |  ____/ __ \
| \  / | |    | |__ | |  | |
| |\/| | |    |  __|| |  | |
| |  | | |____| |   | |__| |
|_|  |_|______|_|    \___\_\
*/

/**
 * @brief Multi Level Feedback Queue Scheduler
 *
 * @param p - the process array
 * @param n - number of processes
 * @param quantum0 - time quantum for queue 0
 * @param quantum1 - time quantum for queue 1
 * @param quantum2 - time quantum for queue 2
 * @param boostTime - time interval for boosting
 */
void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime)
{
    uint64_t start = get_time();
    for (int i = 0; i < n; i++)
    {
        p[i].finished = false;
        p[i].error = false;
        p[i].started = false;
        p[i].burst_time = 0;
    }

    // 3 queues for 3 levels
    ProcessQueue *q2 = createQueue();
    ProcessQueue *q1 = createQueue();
    ProcessQueue *q0 = createQueue();

    ProcessQueue *queues[3] = {q0, q1, q2};

    int i = 0;
    int completed = 0;
    uint64_t boosts = 0;

    GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *GLOB_ERROR = false;

    FILE *f = fopen("result_offline_MLFQ.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);

    for (int i = 0; i < n; i++)
    {
        enqueue(queues[0], &p[i]);
    }

    while (completed < n)
    {
        uint64_t cont_start, cont_end = start;
        uint64_t curr_t = get_time();

        // boost the priority at boostTime intervals
        if (curr_t - start >= boostTime * (boosts + 1))
        {
            boosts++;

            // boost all processes to highest priority
            // boost q[1] to q[0]
            while (!isEmpty(queues[1]))
            {
                Process *p = dequeue(queues[1]);
                enqueue(queues[0], p);
            }
            // boost q[2] to q[0]
            while (!isEmpty(queues[2]))
            {
                Process *p = dequeue(queues[2]);
                enqueue(queues[0], p);
            }
        }
        Process *curr;
        bool queue0, queue1, queue2;
        queue0 = queue1 = queue2 = false;

        // find next process to execute
        if (!isEmpty(queues[0]))
        {
            curr = dequeue(queues[0]);
            queue0 = true;
        }
        else if (!isEmpty(queues[1]))
        {
            curr = dequeue(queues[1]);
            queue1 = true;
        }
        else
        {
            curr = dequeue(queues[2]);
            queue2 = true;
        }

        // continue if process is finished or errored - should not be encountered anyway
        if (curr->finished || curr->error)
        {
            continue;
        }

        // continue process if process is started
        if (curr->started)
        {
            kill(curr->process_id, SIGCONT);
        }
        // start the process if not started
        else
        {
            curr->start_time = get_time();
            curr->started = true;
            curr->process_id = fork();
        }

        // execute the process
        if (curr->process_id == 0)
        {
            char *args[INSIZE];
            int commandSize = strlen(curr->command);
            char command[commandSize + 1];
            strcpy(command, curr->command);
            command[commandSize] = '\0';
            inputParser(command, args);
            if (execvp(args[0], args) == -1)
            {
                *GLOB_ERROR = true;
                exit(0);
            }
            exit(0);
        }
        else
        {
            int quant = 0; // current quantum
            if (queue0)
            {
                quant = quantum0;
            }
            else if (queue1)
            {
                quant = quantum1;
            }
            else
            {
                quant = quantum2;
            }

            cont_start = get_time() - start;
            usleep(quant * 1000);
            curr->burst_time += quant;
            cont_end = get_time() - start;

            // printf("%s|%ld|%ld|%d|%d|%d|%d|%s|%s|%s\n", curr->command, cont_start, cont_end, priority, !isEmpty(queues[0]) ? queues[0]->tail - queues[0]->head + 1 : 0, !isEmpty(queues[1]) ? queues[1]->tail - queues[1]->head + 1 : 0, !isEmpty(queues[2]) ? queues[2]->tail - queues[2]->head + 1 : 0, queue0 ? "Yes" : "No", queue1 ? "Yes" : "No", queue2 ? "Yes" : "No");
            printf("%s|%ld|%ld\n", curr->command, cont_start, cont_end);

            int status = waitpid(curr->process_id, NULL, WNOHANG);
            if (status == 0)
            {
                kill(curr->process_id, SIGSTOP);

                // enqueue the process to the next lower queue
                if (queue0)
                {
                    enqueue(queues[1], curr);
                }
                else if (queue1)
                {
                    enqueue(queues[2], curr);
                }
                else
                {
                    enqueue(queues[2], curr);
                }
            }
            else
            {
                uint64_t end = get_time();
                curr->finished = true;
                completed++;
                if (*GLOB_ERROR)
                {
                    curr->error = true;
                    curr->finished = false;
                    *GLOB_ERROR = false;
                }

                curr->completion_time = end;
                curr->turnaround_time = curr->completion_time - start;
                curr->waiting_time = curr->turnaround_time - curr->burst_time;
                curr->response_time = curr->start_time - start;

                fprintf(f, "%s,%s,%s,%ld,%ld,%ld,%ld\n", curr->command, curr->finished ? "Yes" : "No", curr->error ? "Yes" : "No", curr->burst_time, curr->turnaround_time, curr->waiting_time, curr->response_time);
                fflush(f);
            }
        }
    }
    fclose(f);
    free(q0);
    free(q1);
    free(q2);
}

/*
    ╱|、
    (˚ˎ 。7
    |、˜〵
    じしˍ,)ノ

　 彡 ⌒ ミ
　（´･ω･｀） ))
(( (　つ　ヽ、
　　〉 とノ　)))
　（__ノ^(＿)

　 彡 ⌒ ミ
　（´･ω･｀）
　γ　 と )　))
((( ヽつ 〈
　（＿)^(__)

*/