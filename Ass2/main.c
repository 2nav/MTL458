#include "offline_schedulers.h"
// #include "online_schedulers.h"
#include <stdio.h>

// int main()
// {
//     // create a Process array
//     // freopen("output.txt", "w", stdout);
//     Process p[5];
//     // initialize the Process array
//     p[0].command = "./test";
//     p[1].command = "./test";
//     p[2].command = "./test";
//     p[3].command = "sleep 1.5";
//     p[4].command = "sleep 1";
//     // FCFS(p, 3);
//     RoundRobin(p, 2, 1000);
//     return 0;
// }
void main()
{
    Process p[10];
    p[0].command = "bash sl.sh 1";
    p[1].command = "bash sl.sh 2";
    p[2].command = "bash sl.sh 6";
    p[3].command = "sleeasdap 4";
    p[4].command = "bash sl.sh 4";
    p[5].command = "bash sl.sh 2";
    p[6].command = "bash sl.sh 3";
    p[7].command = "sleeasdap 4";
    p[8].command = "bash sl.sh 2";
    p[9].command = "bash sl.sh 4";
    // MultiLevelFeedbackQueue(p, 10, 100, 200, 300, 3000);
    RoundRobin(p, 10, 100);
}