#pragma once

// Can include any other headers as needed
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h> //check what is the role of this thing
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

int INSIZE = 1024;

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

} Process;

// Function prototypes
void FCFS(Process p[], int n);
void RoundRobin(Process p[], int n, int quantum);
void MultiLevelFeedbackQueue(Process p[], int n, int quantum0, int quantum1, int quantum2);

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
                p[i].error = true;
                exit(0);
            }
            exit(0);
        }
        // Wait for the process to finish
        else
        {
            // waitpid(p[i].process_id, NULL, 0);
            wait(NULL);

            // Update the process parameters
            p[i].finished = true;
            // printf("Process %d ended at %ld\n", p[i].process_id, timebuff.tv_sec * 1000 + timebuff.tv_usec / 1000);
            time_t end = get_time();
            p[i].completion_time = end;
            p[i].turnaround_time = p[i].completion_time - p[i].start_time;
            p[i].waiting_time = 0; // FCFS has no waiting time, TAT = WT
            p[i].response_time = p[i].start_time - start;

            printf("%s|%ld|%ld\n", p[i].command, p[i].start_time - start, p[i].completion_time - start);
            // printf("Completion time: %ld\n Turnaround time: %ld\n Waiting time: %ld\n Response time: %ld\n", p[i].completion_time, p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
            i++;
        }
    }

    // create a csv and dump Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time
    FILE *f = fopen("output.csv", "w");
    fprintf(f, "Command, Finished, Error, Burst Time, Turnaround Time, Waiting Time, Response Time\n");
    for (int i = 0; i < n; i++)
    {
        fprintf(f, "%s, %s, %s, %ld, %ld, %ld, %ld\n", p[i].command, p[i].finished ? "true" : "false", p[i].error ? "true" : "false", p[i].completion_time - p[i].start_time, p[i].turnaround_time, p[i].waiting_time, p[i].response_time);
    }
}

void RoundRobin(Process p[], int n, int quantum)
{
    uint64_t start = get_time();
    p[0].process_id = fork();
    if (p[0].process_id == 0)
    {
        // Execute the process
        p[0].finished = false;
        p[0].error = false;
        char *args[INSIZE];
        int commandSize = strlen(p[0].command);
        char command[commandSize + 1];
        strcpy(command, p[0].command);
        command[commandSize] = '\0';
        inputParser(command, args);

        p[0].start_time = get_time();
        // printf("Process started at %ld\n", p[0].start_time);

        if (execvp(args[0], args) == -1)
        {
            printf("Error executing command\n");
            p[0].error = true;
            exit(0);
        }
        exit(0);
    }
    else
    {
        sleep(2);
        kill(p[0].process_id, SIGSTOP);
        sleep(2);
        kill(p[0].process_id, SIGCONT);
        wait(NULL);
        time_t end = get_time();
        printf("%s|%ld\n", p[0].command, end - start);
    }
}
