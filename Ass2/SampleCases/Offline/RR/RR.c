#include "../../offline_schedulers copy.h"

int main()
{
    int n = 2;
    Process p[n];
    // p[0].command = "./dummy_p 10";
    // p[1].command = "./dummy_p 9";
    p[0].command = "./sl.sh 10";
    p[1].command = "./sl.sh 9";
    // p[2].command = "./dummy_p 2";
    // p[0].command = "sleep 1";
    // p[1].command = "sleep 2";
    // p[2].command = "sleep 3";
    RoundRobin(p, n, 1000);
}