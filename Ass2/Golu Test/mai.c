#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "offline_schedulers.h"
// #include "online_schedulers.h"

int main()
{
    // const char *commands[] = {
    //     "./process.sh p1 100 0.05 outputs/p1.txt",
    //     "./process.sh p2 1000 0.01 outputs/p2.txt",
    //     "./procesd.sh p3 30 0.1 outputs/p3.txt",
    //     "./process.sh p4 30 0.1 outputs/p3.txt",
    //     "./procesd.sh p5 300 0.01 outputs/p3.txt",
    //     "./process.sh p6 10 0.05 outputs/p1.txt"};

    // const char *commands[] = {
    //     "./process 5 p1",
    //     "./process 10 p2",
    //     "./procesd 3 p3",
    //     "./process 3 p4",
    //     "./procesd 7 p5",
    //     "./process 5 p6"};

    const char *commands[] = {
        "./process 1 p1",
        "./process 2 p2",
        "./process 6 p3",
        "./procesd 33 p4",
        "./process 4 p5",
        "./process 2 p6",
        "./process 3 p7",
        "./procesd 1 p8",
        "./process 2 p9",
        "./process 4 p10",
    };

    size_t num_commands = sizeof(commands) / sizeof(commands[0]);

    Process *processes = malloc(num_commands * sizeof(Process));
    if (processes == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < num_commands; ++i)
    {
        processes[i].command = strdup(commands[i]);
        if (processes[i].command == NULL)
        {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
    }
    // FCFS(processes, num_commands);
    // RoundRobin(processes, num_commands, 100);
    MultiLevelFeedbackQueue(processes, num_commands, 100, 200, 300, 3000);
    // ShortestJobFirst();
    // freopen("output.txt", "w", stdout);
    // fflush(stdout);
    // MultiLevelFeedbackQueue(10, 20, 30, 3000);

    for (size_t i = 0; i < num_commands; ++i)
    {
        free(processes[i].command);
    }
    free(processes);

    return 0;
}
