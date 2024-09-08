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

// Function prototypes
void FCFS(Process p[], int n);
void RoundRobin(Process p[], int n, int quantum);
void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2, int boostTime);

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
        char *args[INSIZE];
        int commandSize = strlen(p[i].command);
        char command[commandSize + 1];
        strcpy(command, p[i].command);
        command[commandSize] = '\0';
        inputParser(command, args);

        p[i].start_time = get_time();
        // printf("Process started at %ld\n", p[i].start_time);

        p[i].process_id = fork();
        // printf("Process %d started\n", p[i].process_id);
        if (p[i].process_id == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                printf("Error executing command\n");
                // p[i].error = true;
                *GLOB_ERROR = true;
                exit(0);
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
            // printf("Process %d ended at %ld\n", p[i].process_id, timebuff.tv_sec * 1000 + timebuff.tv_usec / 1000);
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
        // printf("process %d status = %d\n", i, p[i].finished);
        // printf("i = %d\n", i);
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
            // printf("Process %s started at %ld\n", p[i].command, p[i].start_time);
            p[i].started = true;
            p[i].process_id = fork();
        }

        // printf("Process %d started\n", p[i].process_id);
        if (p[i].process_id == 0)
        {
            char *args[INSIZE];
            int commandSize = strlen(p[i].command);
            char command[commandSize + 1];
            strcpy(command, p[i].command);
            command[commandSize] = '\0';
            inputParser(command, args);
            // printf("Process %s started at %ld\n", p[i].command, p[i].start_time);
            if (execvp(args[0], args) == -1)
            {
                *GLOB_ERROR = true;
                // printf("Error executing command\n");
                exit(0);
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
                // printf("status = %d\n", status);
                uint64_t end = get_time();
                // printf("Status: %d and time passed %ld\n", status, end - start);
                // printf("i = %d\n", i);
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
    int quanta[3] = {quantum2, quantum1, quantum0}; // initiailly implemented with 2 as highest, switching makes less readable somewhat
    int MLFQ_current = 2;

    int i = 0;
    int completed = 0;
    int boosts = 0;

    GLOB_ERROR = mmap(NULL, sizeof *GLOB_ERROR, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *GLOB_ERROR = false;

    FILE *f = fopen("result_offline_MLFQ.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    fflush(f);

    while (completed < n)
    {
        uint64_t cont_start, cont_end = start;
        if (i == n && MLFQ_current > 0)
        {
            MLFQ_current--;
        }
        uint64_t curr_t = get_time();
        if (curr_t - start >= boostTime * (boosts + 1) && MLFQ_current == 0)
        {
            MLFQ_current = 2;
            boosts++;
        }
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
                exit(0);
            }
            exit(0);
        }
        else
        {
            cont_start = get_time() - start;
            usleep(quanta[MLFQ_current] * 1000);
            p[i].burst_time += quanta[MLFQ_current];
            cont_end = get_time() - start;
            printf("%s|%ld|%ld\n", p[i].command, cont_start, cont_end);
            int status = waitpid(p[i].process_id, NULL, WNOHANG);
            if (status == 0)
            {

                kill(p[i].process_id, SIGSTOP);
            }
            else
            {
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
    fclose(f);
}