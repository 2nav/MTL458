#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#define INSIZE 100
int main(int argc, char *argv[])
{
    printf("hello (pid:%d)\n", (int)getpid());
    while (1)
    {
        // Taking string input in C - https://stackoverflow.com/a/58703958/23151163
        char str[INSIZE];
        printf("MTL458 >");
        scanf(" %99[^\n]", str);

        // termination on user interrupt
        if (strcmp(str, "exit") == 0)
        {
            printf("Exiting shell\n");
            break;
        }

        // parsing the string into space separated tokens for args
        char *args[INSIZE];
        char *token = strtok(str, " ");
        int i = 0;
        while (token != NULL)
        {
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i] = '\0';

        // printf("args: %s\n", args[0]);

        printf("hello (pid:%d)\n", (int)getpid());
        int rc = fork();
        if (rc < 0)
        {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc == 0)
        { // child (new process)
            printf("child (pid:%d)\n", (int)getpid());
            char *myargs[2];
            // myargs[0] = strdup("ls"); // program: "wc"
            // myargs[1] = strdup("p3.c"); // arg: input file
            // myargs[1] = NULL;
            // mark end of array
            // execvp(myargs[0], myargs); // runs word count
            // int n = strlen(str);
            // printf("n: %d\n", n);
            // char *args = (char *)str;
            // printf("args: %s\n", args);
            execvp(args[0], args);
            printf("this shouldn’t print out\n");
        }
        else
        {
            // parent goes down this path
            int rc_wait = wait(NULL);
            printf("parent of %d (rc_wait:%d) (pid:%d)\n",
                   rc, rc_wait, (int)getpid());
        }
    }
    return 0;
}