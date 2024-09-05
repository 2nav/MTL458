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

int INSIZE = 1024;
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
    bool started;
    int process_id;

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

// Function prototypes
void ShortestJobFirst();
void ShortestRemainingTimeFirst();
void MultiLevelFeedbackQueue(int quantum0, int quantum1, int quantum2, int boostTime);

void ShortestJobFirst()
{
}