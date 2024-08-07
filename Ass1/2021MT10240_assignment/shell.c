#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define INSIZE 2049
#define HISTORYSIZE 10
int COUNT = 0;
char HOME[INSIZE];
char DIRSUFFIX[INSIZE];
char HOME1[INSIZE * 2];
char PWD[INSIZE * 2];

/**
 * @brief Function to print history of commands
 *
 * @param history
 */
void printHistory(char history[HISTORYSIZE][INSIZE])
{
    if (COUNT <= HISTORYSIZE)
    {
        for (int i = 0; i < COUNT; i++)
        {
            printf("%d: %s\n", i + 1, history[i]);
        }
        return;
    }

    for (int i = 0; i < HISTORYSIZE; i++)
    {
        printf("%d: %s\n", i + 1, history[(i + COUNT) % HISTORYSIZE]);
    }
}

void changeDirectory(char *args[INSIZE])
{
    if (args[1] == NULL)
    {
        chdir(PWD);
        return;
    }
    if (args[1][0] == '~')
    {
        chdir(HOME1);
        strcpy(PWD, HOME1);
        return;
    }
    else
    {
        char *temp = (char *)malloc(INSIZE);
        strcpy(temp, PWD);
        strcat(temp, "/");
        strcat(temp, args[1]);
        if (chdir(temp) == 0)
        {
            strcpy(PWD, temp);
        }
        else
        {
            printf("Invalid Directory\n");
        }
    }
    return;
}

int main(int argc, char *argv[])
{
    // get current working directory
    if (getcwd(HOME, sizeof(HOME)) != NULL)
    {
        printf("Current working dir: %s\n", HOME);
    }
    else
    {
        perror("getcwd() error");
        return 1;
    }
    strcpy(DIRSUFFIX, "/home/user");
    strcat(HOME1, HOME);
    strcat(HOME1, DIRSUFFIX);
    printf("PWD: %s\n", HOME1);
    strcpy(PWD, HOME1);
    chdir(HOME1);

    printf("hello (pid:%d)\n", (int)getpid());
    while (1)
    {
        // Taking string input in C - https://stackoverflow.com/a/58703958/23151163
        // chdir(PWD);
        char str[INSIZE];
        char history[HISTORYSIZE][INSIZE]; // to store history of commands

        printf("MTL458 >");
        scanf(" %99[^\n]", str);

        // storing the command in history
        strcpy(history[COUNT % HISTORYSIZE], str);
        COUNT++;

        // termination on user interrupt
        if (strcmp(str, "exit") == 0)
        {
            printf("Exiting shell\n");
            exit(0);
        }

        // printing history and continue
        if (strcmp(str, "history") == 0)
        {
            printHistory(history);
            continue;
        }

        int pipeFlag = 0; // Need <stdbool.h> for true and false :-(

        // parsing the string into space separated tokens for args
        // https://stackoverflow.com/a/3890186/23151163
        char *args[INSIZE];
        char *token = strtok(str, " ");
        int i = 0, commandSize1 = 0, commandSize2 = 0; // commandSize1 is for first command and commandSize2 is for second command
        while (token != NULL)
        {
            if (pipeFlag)
            {
                commandSize2++;
            }
            else
            {
                commandSize1++;
            }
            if (strcmp(token, "|") == 0)
            {
                pipeFlag = 1;
            }
            args[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        args[i] = '\0';

        // handling cd
        if (strcmp(args[0], "cd") == 0)
        {
            changeDirectory(args);
            continue;
        }
        // parse pipe commands
        char *args1[INSIZE], *args2[INSIZE];
        if (pipeFlag)
        {

            for (int i = 0; i < commandSize1 - 1; i++)
            {
                args1[i] = args[i];
                if (i == commandSize1 - 2)
                {
                    args1[i + 1] = '\0';
                }
                printf("args1[%d]: %s\n", i, args1[i]);
            }

            for (int i = 0; i <= commandSize2; i++)
            {
                args2[i] = args[commandSize1 + i];
                // if (i == commandSize2 - 1)
                // {
                //     args2[i + 1] = '\0';
                // }
                printf("args2[%d]: %s\n", i, args2[i]);
            }
        }

        printf("commandSize1: %d\n commandSize2: %d\n", commandSize1, commandSize2);

        printf("hello (pid:%d)\n", (int)getpid());
        int rc = fork();
        if (rc < 0)
        {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc == 0 && pipeFlag == 0)
        { // child (new process)
            printf("child (pid:%d)\n", (int)getpid());
            // char *myargs[2];
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
            printf("Invalid Command\n");
        }
        else if (rc == 0 && pipeFlag == 1)
        {
            printf("child (pid:%d)\n", (int)getpid());

            int pipefd[2];
            if (pipe(pipefd) == -1) // https://www.man7.org/linux/man-pages/man2/pipe.2.html
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            int rc2 = fork();
            if (rc2 < 0)
            {
                // fork failed; exit
                fprintf(stderr, "fork failed\n");
                exit(1);
            }
            else if (rc2 == 0)
            {
                // child (new process)
                printf("child (pid:%d)\n", (int)getpid());

                close(STDOUT_FILENO); // close normal stdout
                dup(pipefd[1]);       // make stdout go to write end of pipe
                close(pipefd[0]);     // close read end of pipe

                execvp(args1[0], args1);
                printf("Invalid Command\n");
                exit(1);
            }
            else
            {
                // parent goes down this path
                int rc_wait = wait(NULL);
                printf("parent of %d (rc_wait:%d) (pid:%d)\n",
                       rc, rc_wait, (int)getpid());
                close(STDIN_FILENO); // close normal stdin
                dup(pipefd[0]);      // make stdin come from read end of pipe
                close(pipefd[1]);    // close write end of pipe

                execvp(args2[0], args2);
                printf("Invalid Command\n");
                exit(1);
            }
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
/*
    ╱|、
    (˚ˎ 。7
    |、˜〵
    じしˍ,)ノ
*/