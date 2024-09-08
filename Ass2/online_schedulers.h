#pragma once

// Can include any other headers as needed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>

#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>

#define INSIZE 1024
#define MAX_PROCESSES 50
static bool *GLOB_ERROR;

typedef struct
{
    char *command;

    bool finished;
    bool error;
    uint64_t start_time;
    uint64_t completion_time;
    uint64_t turnaround_time;
    uint64_t waiting_time;
    uint64_t response_time;
    uint64_t burst_time;
    uint64_t arrival_time;

    uint64_t average_time;
    uint8_t priority;
    bool started;
    int process_id;
    char *args[INSIZE];
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
    Process *processes[MAX_PROCESSES];
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

/**
 * @brief Set the average time for process, 1000ms if not yet appeared(and finished), average burst time otherwise
 *
 * @param p
 * @param index
 * @param proc_count
 */
void set_average_time(Process *p, int index, uint8_t proc_count)
{
    int count = 0;
    int total_time = 0;
    for (uint8_t i = 0; i < proc_count; i++)
    {
        if (strcmp(p[index].command, p[i].command) == 0 && p[i].finished && !p[i].error)
        {
            total_time += p[i].burst_time;
            count++;
        }
    }
    if (count == 0)
    {
        p[index].average_time = 1000;
    }
    else
    {
        p[index].average_time = total_time / count;
    }
}

/**
 * @brief Set the priority of the process according to the average burst time
 *
 * @param p
 * @param index
 * @param proc_count
 * @param quantum0
 * @param quantum1
 * @param quantum2
 * @param queues
 */
void set_priority_MLFQ(Process *p, int index, uint8_t proc_count, int quantum0, int quantum1, int quantum2, ProcessQueue *queues[3])
{
    int count = 0;
    int total_time = 0;
    for (uint8_t i = 0; i < proc_count; i++)
    {
        if (strcmp(p[index].command, p[i].command) == 0 && p[i].finished && !p[i].error)
        {
            total_time += p[i].burst_time;
            count++;
        }
    }
    if (count > 0)
    {
        int average_time = total_time / count;
        if (average_time <= quantum0)
        {
            // printf("command: %s, average time: %d\n", p[index].command, average_time);
            p[index].priority = 0;
            enqueue(queues[0], &p[index]);
        }
        else if (average_time <= quantum1)
        {
            p[index].priority = 1;
            enqueue(queues[1], &p[index]);
        }
        else
        {
            p[index].priority = 2;
            enqueue(queues[2], &p[index]);
        }
    }
    else
    {
        p[index].priority = 1;
        enqueue(queues[1], &p[index]);
    }
}

// Function prototypes
void ShortestJobFirst();
void ShortestRemainingTimeFirst();
void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime);

/*
 _____      _ ______
 / ____|    | |  ____|
| (___      | | |__
 \___ \ _   | |  __|
 ____) | |__| | |
|_____/ \____/|_|
*/

/**
 * @brief Shortest Job First Scheduler
 */
void ShortestJobFirst()
{
    Process p[MAX_PROCESSES];

    FILE *f = fopen("result_online_SJF.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);

    uint8_t proc_count = 0;
    uint64_t start = get_time();

    char comm_buff[INSIZE];

    // global error flag
    GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *GLOB_ERROR = false;

    // non blocking input
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK); // I do not understand a single word written here
    while (1)
    {
        uint64_t proc_start, proc_end;

        // read input
        while (proc_count < MAX_PROCESSES)
        {
            if (fgets(comm_buff, INSIZE, stdin) == NULL)
            {
                break;
            }
            comm_buff[strcspn(comm_buff, "\n")] = 0;

            p[proc_count].command = strdup(comm_buff);
            p[proc_count].arrival_time = get_time() - start;
            p[proc_count].started = false;
            p[proc_count].error = false;
            p[proc_count].finished = false;

            proc_count++;
        }

        uint64_t min_time = UINT64_MAX;
        int min_index = -1;

        // find the remaining unstarted process with the minimum average time
        for (int i = 0; i < proc_count; i++)
        {
            if (!p[i].started)
            {
                set_average_time(p, i, proc_count);
                if (p[i].average_time < min_time)
                {
                    min_time = p[i].average_time;
                    min_index = i;
                }
            }
        }

        // if no unstarted process is found, continue
        if (min_index == -1)
        {
            continue;
        }

        // run the process
        p[min_index].start_time = get_time() - start;
        p[min_index].started = true;
        proc_start = get_time() - start;
        p[min_index].process_id = fork();
        if (p[min_index].process_id == 0)
        {
            char *args[INSIZE];
            int commandSize = strlen(p[min_index].command);
            char command[commandSize + 1];
            strcpy(command, p[min_index].command);
            command[commandSize] = '\0';
            inputParser(command, args);

            execvp(args[0], args);
            *GLOB_ERROR = true;
            exit(1);
        }
        else
        {
            waitpid(p[min_index].process_id, NULL, 0);
            proc_end = get_time() - start;
            printf("%s|%ld|%ld\n", p[min_index].command, proc_start, proc_end);
            p[min_index].completion_time = get_time() - start;
            p[min_index].burst_time = p[min_index].completion_time - p[min_index].start_time;
            p[min_index].turnaround_time = p[min_index].completion_time - p[min_index].arrival_time;
            p[min_index].waiting_time = p[min_index].turnaround_time - p[min_index].burst_time;
            p[min_index].response_time = p[min_index].start_time - p[min_index].arrival_time;

            if (*GLOB_ERROR)
            {
                p[min_index].error = true;
                p[min_index].finished = false;
            }
            else
            {
                p[min_index].error = false;
                p[min_index].finished = true;
            }
            *GLOB_ERROR = false;
            fprintf(f, "%s,%s,%s,%ld,%ld,%ld,%ld\n", p[min_index].command, p[min_index].finished ? "Yes" : "No", p[min_index].error ? "Yes" : "No", p[min_index].burst_time, p[min_index].turnaround_time, p[min_index].waiting_time, p[min_index].response_time);
            fflush(f);
        }
    }
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
 * @param quantum0 - time quantum for queue 0
 * @param quantum1 - time quantum for queue 1
 * @param quantum2 - time quantum for queue 2
 * @param boostTime - time after which all processes are moved to queue 0
 */
void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime)
{
    Process p[MAX_PROCESSES];
    FILE *f = fopen("result_online_MLFQ.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);

    uint8_t proc_count = 0;
    uint64_t start = get_time();

    char comm_buff[INSIZE];

    // global error flag
    GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *GLOB_ERROR = false;

    // the queue flags
    bool queue0 = false;
    bool queue1 = false;
    bool queue2 = false;

    // the queues
    ProcessQueue *q0 = createQueue();
    ProcessQueue *q1 = createQueue();
    ProcessQueue *q2 = createQueue();

    // array of queues
    ProcessQueue *queues[3] = {q0, q1, q2};

    uint64_t boosts = 0;

    // non blocking input
    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK); // I do not understand a single word written here
    while (1)
    {
        uint64_t cont_start, cont_end;
        int prev_proc = 0;

        // read input
        while (proc_count < MAX_PROCESSES)
        {
            if (fgets(comm_buff, INSIZE, stdin) == NULL)
            {
                break;
            }
            comm_buff[strcspn(comm_buff, "\n")] = 0;
            p[proc_count].command = strdup(comm_buff);
            p[proc_count].arrival_time = get_time() - start;
            p[proc_count].started = false;
            p[proc_count].error = false;
            p[proc_count].finished = false;
            p[proc_count].burst_time = 0;

            // update the priority according to burst time
            set_priority_MLFQ(p, proc_count, proc_count, quantum0, quantum1, quantum2, queues);

            proc_count++;
        }

        // int curr_proc = -1;
        int curr_quantum = 0;
        queue0 = false;
        queue1 = false;
        queue2 = false;

        uint64_t curr_t = get_time() - start;

        // boost the processes periodically
        if (curr_t > (boosts + 1) * boostTime)
        {
            boosts++;
            for (int i = 0; i < proc_count; i++)
            {
                if (!p[i].finished && !p[i].error)
                {
                    p[i].priority = 0;
                }
            }

            // Move all processes of queue 1 to queue 0
            while (!isEmpty(q1))
            {
                Process *p = dequeue(q1);
                p->priority = 0;
                enqueue(q0, p);
                // printf("Size of q0: %d, q1: %d\n", size(q0), size(q1));
            }

            // Move all processes of queue 2 to queue 0
            while (!isEmpty(q2))
            {
                Process *p = dequeue(q2);
                p->priority = 0;
                enqueue(q0, p);
            }
        }

        queue0 = queue1 = queue2 = false;
        Process *curr;
        int priority = 0;
        if (!isEmpty(q0))
        {
            queue0 = true;
            curr = dequeue(q0);
            curr_quantum = quantum0;
            priority = 0;
        }
        else if (!isEmpty(q1))
        {
            queue1 = true;
            curr = dequeue(q1);
            curr_quantum = quantum1;
            priority = 1;
        }
        else if (!isEmpty(q2))
        {
            queue2 = true;
            curr = dequeue(q2);
            curr_quantum = quantum2;
            priority = 2;
        }

        // continue if no process is available
        if (!queue0 && !queue1 && !queue2)
        {
            continue;
        }

        // resume the process if it was previously started and paused
        if (curr->started)
        {
            kill(curr->process_id, SIGCONT);
        }
        // start the process if it is not started
        else
        {
            curr->start_time = get_time() - start;
            curr->started = true;
            curr->process_id = fork();
        }
        // run the process
        if (curr->process_id == 0)
        {
            char *args[INSIZE];
            int commandSize = strlen(curr->command);
            char command[commandSize + 1];
            strcpy(command, curr->command);
            command[commandSize] = '\0';
            inputParser(command, args);

            execvp(args[0], args);

            *GLOB_ERROR = true;
            exit(1);
        }
        else
        {
            cont_start = get_time() - start;
            usleep(curr_quantum * 1000);
            curr->burst_time += curr_quantum;
            cont_end = get_time() - start;
            // curr->burst_time += cont_end - cont_start;

            printf("%s|%ld|%ld\n", curr->command, cont_start, cont_end);
            // printf("%s|%ld|%ld|%d\n", curr->command, cont_start, cont_end, priority);

            // check if the process is finished
            int status = waitpid(curr->process_id, NULL, WNOHANG);
            if (status == 0)
            {
                kill(curr->process_id, SIGSTOP);

                // move to the next lower queue if not finished
                if (queue0)
                {
                    enqueue(queues[1], curr);
                    // printf("Enqueued %s to q1, q1 size = %d\n", curr->command, );
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
                curr->completion_time = get_time() - start;
                // curr->burst_time = curr->completion_time - curr->start_time;
                curr->turnaround_time = curr->completion_time - curr->arrival_time;
                curr->waiting_time = curr->turnaround_time - curr->burst_time;
                curr->response_time = curr->start_time - curr->arrival_time;
                if (*GLOB_ERROR)
                {
                    curr->error = true;
                    curr->finished = false;
                }
                else
                {
                    curr->error = false;
                    curr->finished = true;
                }
                *GLOB_ERROR = false;
                fprintf(f, "%s,%s,%s,%ld,%ld,%ld,%ld\n", curr->command, curr->finished ? "Yes" : "No", curr->error ? "Yes" : "No", curr->burst_time, curr->turnaround_time, curr->waiting_time, curr->response_time);
                fflush(f);
            }
        }
    }
}

/*
      ,-'"""`-.
    ,'         `.
   /        `    \
  (    /          )
  |             " |
  (               )
 `.\\          \ /
   `:.     , \ ,\ _
 hh  `:-.___,-`-.{\)
       `.        |/ \
         `.        \ \
           `-.     _\,)
              `.  |,-||
                `.|| ||


    One must imagine Scheduler happy.
 */