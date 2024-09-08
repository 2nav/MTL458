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

void set_priority_MLFQ(Process *p, int index, uint8_t proc_count, int quantum0, int quantum1, int quantum2)
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
        }
        else if (average_time <= quantum1)
        {
            p[index].priority = 1;
        }
        else
        {
            p[index].priority = 2;
        }
    }
}

// Function prototypes
void ShortestJobFirst();
void ShortestRemainingTimeFirst();
void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime);

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
            // char *args[INSIZE];
            // int commandSize = strlen(p[proc_count].command);
            // char command[commandSize+1];
            // strcpy(command, p[proc_count].command);
            // // command[commandSize] = '\0';
            // printf("Command: %s\n", command);
            // inputParser(command, p[proc_count].args);

            // int it = 0;
            // while (p[proc_count].args[it] != NULL)
            // {
            //     printf("%s ", p[proc_count].args[it]);
            //     it++;
            // }
            // printf("\n");

            p[proc_count].arrival_time = get_time() - start;
            p[proc_count].started = false;
            p[proc_count].error = false;
            p[proc_count].finished = false;

            proc_count++;
            // printf("Command: %s\n", p[proc_count - 1].command);
        }

        uint64_t min_time = UINT64_MAX;
        int min_index = -1;
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
            // char *args[INSIZE];
            // int it = 0;
            // printf("Executing: %s\n", p[min_index].command);
            // while (p[min_index].args[it] != NULL)
            // {
            //     printf("%s\n", p[min_index].args[it]);
            //     it++;
            // }
            // args[it] = NULL;
            char *args[INSIZE];
            int commandSize = strlen(p[min_index].command);
            char command[commandSize + 1];
            strcpy(command, p[min_index].command);
            command[commandSize] = '\0';
            inputParser(command, args);

            execvp(args[0], args);

            // int it = 0;
            // while (p[min_index].args[it] != NULL)
            // {
            //     printf("%s ", p[min_index].args[it]);
            //     it++;
            // }
            // execvp(p[min_index].args[0], p[min_index].args);
            // printf("error in execvp\n");
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

void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime)
{
    Process p[MAX_PROCESSES];
    FILE *f = fopen("result_online_MLFQ.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);
    printf("MLFQ\n");

    uint8_t proc_count = 0;
    uint64_t start = get_time();

    char comm_buff[INSIZE];

    // global error flag
    GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *GLOB_ERROR = false;

    // the queues as 3 flags
    bool queue0 = false;
    bool queue1 = false;
    bool queue2 = false;

    int boosts = 0;

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
            p[proc_count].priority = 1;

            printf("Command: %s\n", p[proc_count].command);
            proc_count++;
        }
        // printf("here at time %ld\n", get_time() - start);

        // printf("prc count %d\n", proc_count);
        int curr_proc = -1;
        int curr_quantum = 0;
        queue0 = false;
        queue1 = false;
        queue2 = false;

        // update the priority according to burst time
        for (int i = 0; i < proc_count; i++)
        {
            set_priority_MLFQ(p, i, proc_count, quantum0, quantum1, quantum2);
        }

        uint64_t curr_t = get_time() - start;
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
        }

        for (int i = 0; i < proc_count; i++)
        {
            int buff = (prev_proc + 1 + i) % proc_count;
            if (!p[buff].finished && !p[buff].error)
            {
                if (p[buff].priority == 0 && !queue0)
                {
                    queue0 = true;
                    curr_proc = buff;
                    curr_quantum = quantum0;
                }
                else if (p[buff].priority == 1 && !queue0 && !queue1)
                {
                    queue1 = true;
                    curr_proc = buff;
                    curr_quantum = quantum1;
                }
                else if (p[buff].priority == 2 && !queue0 && !queue1 && !queue2)
                {
                    queue2 = true;
                    curr_proc = buff;
                    curr_quantum = quantum2;
                }
            }
        }
        if (curr_proc == -1)
        {
            continue;
        }
        // printf("Executing: %s\n", p[curr_proc].command);
        if (p[curr_proc].started)
        {
            kill(p[curr_proc].process_id, SIGCONT);
        }
        else
        {
            p[curr_proc].start_time = get_time() - start;
            p[curr_proc].started = true;
            p[curr_proc].process_id = fork();
        }
        if (p[curr_proc].process_id == 0)
        {
            char *args[INSIZE];
            int commandSize = strlen(p[curr_proc].command);
            char command[commandSize + 1];
            strcpy(command, p[curr_proc].command);
            command[commandSize] = '\0';
            inputParser(command, args);

            execvp(args[0], args);

            *GLOB_ERROR = true;
            exit(1);
        }
        else
        {
            // fprintf(f, "Current priority %d , Curremt time %ld\n", p[curr_proc].priority, get_time() - start);
            // fflush(f);
            cont_start = get_time() - start;
            usleep(curr_quantum * 1000);
            p[curr_proc].burst_time += curr_quantum;
            cont_end = get_time() - start;

            printf("%s|%ld|%ld\n", p[curr_proc].command, cont_start, cont_end);
            prev_proc = curr_proc;

            p[curr_proc].priority = p[curr_proc].priority != 2 ? p[curr_proc].priority + 1 : 2;

            int status = waitpid(p[curr_proc].process_id, NULL, WNOHANG);
            if (status == 0)
            {
                kill(p[curr_proc].process_id, SIGSTOP);
            }
            else
            {
                p[curr_proc].completion_time = get_time() - start;
                p[curr_proc].burst_time = p[curr_proc].completion_time - p[curr_proc].start_time;
                p[curr_proc].turnaround_time = p[curr_proc].completion_time - p[curr_proc].arrival_time;
                p[curr_proc].waiting_time = p[curr_proc].turnaround_time - p[curr_proc].burst_time;
                p[curr_proc].response_time = p[curr_proc].start_time - p[curr_proc].arrival_time;
                if (*GLOB_ERROR)
                {
                    p[curr_proc].error = true;
                    p[curr_proc].finished = false;
                }
                else
                {
                    p[curr_proc].error = false;
                    p[curr_proc].finished = true;
                }
                *GLOB_ERROR = false;
                fprintf(f, "%s,%s,%s,%ld,%ld,%ld,%ld\n", p[curr_proc].command, p[curr_proc].finished ? "Yes" : "No", p[curr_proc].error ? "Yes" : "No", p[curr_proc].burst_time, p[curr_proc].turnaround_time, p[curr_proc].waiting_time, p[curr_proc].response_time);
                fflush(f);
            }
        }
    }
}
