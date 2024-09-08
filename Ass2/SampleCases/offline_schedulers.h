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

#define MAX_QUEUE_SIZE 100

uint64_t global_time = 0;

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
    uint64_t arrival_time;
    uint64_t burst_time;
    bool started;
    int process_id;
    pid_t actual_pid;
    bool paused;
} Process;

uint64_t get_current_time()
{

    struct timeval timebuff;
    gettimeofday(&timebuff, NULL);
    return timebuff.tv_sec * 1000 + timebuff.tv_usec / 1000;
}

void log_process_details(Process p)
{
    uint64_t burst_time = p.burst_time;
    uint64_t turnaround_time = p.completion_time - p.arrival_time;
    uint64_t waiting_time = turnaround_time - burst_time;
    uint64_t response_time = p.start_time - p.arrival_time;

    FILE *fp = fopen("result_offline_scheduler.csv", "a");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s,%s,%s,%llu,%llu,%llu,%llu\n",
            p.command,
            p.finished ? "Yes" : "No",
            p.error ? "Yes" : "No",
            burst_time,
            turnaround_time,
            waiting_time,
            response_time);

    fclose(fp);
}

void parse_command(const char *command, char *args[])
{
    char *cmd = strdup(command);
    char *token = strtok(cmd, " \n");
    int i = 0;
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;
}

int total_time = 0;
void execute_process_for_time(Process *p, uint64_t time_slice)
{
    int st = get_current_time();
    if (!p->started)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            char *args[256];
            parse_command(p->command, args);
            if (execvp(args[0], args) < 0)
            {
                perror("execvp failed");
                p->error = true;
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0)
        {
            p->arrival_time = 0;
            p->actual_pid = pid;
            p->start_time = get_current_time() - global_time;
            p->started = true;
            p->paused = false;
            p->finished = 0;
            p->burst_time = 0;
        }
        else
        {
            perror("fork failed");
        }
    }
    else if (p->paused)
    {
        kill(p->actual_pid, SIGCONT);
        p->paused = false;
    }
    usleep(time_slice * 1000);
    int status;
    pid_t result = waitpid(p->actual_pid, &status, WNOHANG);
    if (result == 0)
    {
        kill(p->actual_pid, SIGSTOP);
        p->paused = true;
        p->burst_time += get_current_time() - st;
        printf("pausedddddd");

        // Printing the process switch and time taken for each process to run
        printf("Process id:%d | Start:%d | end:%d | Completed:%d |total time:%d\n", p->actual_pid, total_time, total_time + get_current_time() - st, 0, p->burst_time);
        total_time += get_current_time() - st;
    }
    else if (result == -1)
    {
        printf("dkshdakdsk");
        perror("waitpid failed");
        p->error = true;
    }
    else
    {
        if (WIFEXITED(status))
        {
            p->completion_time = get_current_time() - global_time;
            p->finished = true;
            p->burst_time += get_current_time() - st;
            printf("finished");

            // Printing the process switch and time taken for each process to run
            printf("Process id:%d | Start:%d | end:%d | Completed:%d | total time:%d\n", p->actual_pid, total_time, total_time + get_current_time() - st, 1, p->burst_time);
            total_time += get_current_time() - st;
        }
        else
        {
            printf("Fuck");
        }
    }
}

void RoundRobin(Process p[], int n, uint64_t quantum)
{
    int r_p = n;
    global_time = get_current_time();

    while (r_p > 0)
    {
        for (int i = 0; i < n; i++)
        {
            if (!p[i].finished)
            {
                execute_process_for_time(&p[i], quantum);
                printf("finished: %d\n", p[i].finished);
                printf("remaiminig:%d\n", r_p);
                if (p[i].finished)
                {
                    r_p--;
                    log_process_details(p[i]);
                }
            }
        }
    }
}
