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
    struct timespec ts;
    // clock_gettime(CLOCK_MONOTONIC, &ts);
    gettimeofday(&ts, NULL);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000); // Convert to milliseconds
}

void log_process_details(Process p)
{
    p.turnaround_time = p.completion_time - p.arrival_time;
    p.waiting_time = p.turnaround_time - p.burst_time;
    p.response_time = p.start_time - p.arrival_time;

    FILE *fp = fopen("result_offline_scheduler.csv", "a");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0)
    {
        fprintf(fp, "Command,Finished,Error,Burst Time,Turnaround Time,Waiting Time,Response Time\n");
    }
    fseek(fp, 0, SEEK_END);

    fprintf(fp, "%s,%s,%s,%llu,%llu,%llu,%llu\n",
            p.command,
            p.finished ? "Yes" : "No",
            p.error ? "Yes" : "No",
            p.burst_time,
            p.turnaround_time,
            p.waiting_time,
            p.response_time);

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

uint64_t total_time = 0;

void execute_process_for_time(Process *p, uint64_t time_slice)
{
    uint64_t st = get_current_time();
    if (!p->started)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            char *args[256];
            parse_command(p->command, args);
            if (execvp(args[0], args) < 0)
            {
                exit(100);
            }
        }
        else if (pid > 0)
        {
            p->arrival_time = global_time;
            p->actual_pid = pid;
            p->start_time = get_current_time();
            p->started = true;
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
        p->burst_time = p->burst_time + get_current_time() - st;

        printf("%s | %llu | %llu\n", p->command, total_time, total_time + get_current_time() - st);
        total_time = total_time + get_current_time() - st;
    }
    else if (result == -1)
    {
        p->error = true;
    }
    else
    {
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) == 100)
            {
                p->finished = false;
                p->error = true;
            }
            else
            {
                p->finished = true;
            }
            p->completion_time = get_current_time();
            p->burst_time = p->burst_time + get_current_time() - st;

            printf("%s | %llu | %llu\n", p->command, total_time, total_time + get_current_time() - st);
            total_time = total_time + get_current_time() - st;
        }
    }
}

void initialise_process(Process p[], int n)
{
    for (int i = 0; i < n; i++)
    {
        p[i].paused = false;
        p[i].burst_time = 0;
        p[i].waiting_time = 0;
        p[i].finished = false;
        p[i].error = false;
        p[i].started = false;
        p[i].start_time = 0;
        p[i].completion_time = 0;
        p[i].turnaround_time = 0;
        p[i].response_time = 0;
        p[i].arrival_time = 0;
        p[i].actual_pid = 0;
    }
}

void RoundRobin(Process p[], int n, uint64_t quantum)
{
    int r_p = n;
    global_time = get_current_time();
    initialise_process(p, n);

    while (r_p > 0)
    {
        for (int i = 0; i < n; i++)
        {
            if (!p[i].finished && !p[i].error)
            {
                execute_process_for_time(&p[i], quantum);
                if (p[i].finished || p[i].error)
                {
                    r_p--;
                    log_process_details(p[i]);
                }
            }
        }
    }
}
