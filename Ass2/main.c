#include "offline_schedulers.h"
// #include "online_schedulers.h"
#include <stdio.h>

int main()
{
    // create a Process array
    Process p[3];
    // initialize the Process array
    p[0].command = "wait 2";
    p[1].command = "wait 1";
    p[2].command = "wait 3";
    FCFS(p, 3);
    return 0;
}