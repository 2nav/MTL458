#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define INSIZE 2049
#define HISTORYSIZE 2048
int COUNT = 0;
char HOME[INSIZE];
char HOME1[INSIZE * 2];
char PWD[INSIZE * 2];  // stores present working directory
char PRWD[INSIZE * 2]; // stores previous working directory

int pipeFlag = 0;                       // Need <stdbool.h> for true and false :-(
int commandSize1 = 0, commandSize2 = 0; // commandSize1 is for first command and commandSize2 is for second command

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
            printf("%s\n", history[i]);
        }
        return;
    }

    for (int i = 0; i < HISTORYSIZE; i++)
    {
        printf("%s\n", history[(i + COUNT) % HISTORYSIZE]);
    }
}

/**
 * @brief Function to change directory
 *
 * @param args
 */
void changeDirectory(char *args[INSIZE])
{
    // getcwd(PWD, sizeof(PWD));

    // goes to home directory, saves current and previous directory
    if (args[1] == NULL)
    {
        strcpy(PRWD, PWD);
        chdir(HOME1);
        strcpy(PWD, HOME1);
        return;
    }

    // same as above, but with ~
    if (args[1][0] == '~')
    {
        strcpy(PRWD, PWD);
        chdir(HOME1);
        strcpy(PWD, HOME1);
        return;
    }

    // goes to previous directory, stores current directory in PRWD and previous directory in PWD
    else if (args[1][0] == '-')
    {
        // printf("PRWD: %s\n", PRWD);
        if (chdir(PRWD) < 0)
        {
            printf("Invalid Directory\n");
            return;
        }
        char *temp = (char *)malloc(INSIZE * 2);
        strcpy(temp, PWD);
        strcpy(PWD, PRWD);
        strcpy(PRWD, temp);
        free(temp);
    }

    // goes to the directory specified in args[1]
    else
    {
        char *temp = (char *)malloc(INSIZE * 2);
        strcpy(temp, PWD);
        strcat(temp, "/");
        strcat(temp, args[1]);
        if (chdir(temp) == 0)
        {
            strcpy(PRWD, PWD);
            strcpy(PWD, temp);
            printf("PWD: %s\n", PWD);
        }
        else
        {
            printf("Invalid Directory\n");
        }
        free(temp);
    }
    return;
}

/**
 * @brief Function to parse input string into space separated tokens and check for pipe
 *
 * @param str
 * @param args
 */
void inputParser(char *str, char *args[INSIZE])
{
    char *token = strtok(str, " ");
    int i = 0;
    commandSize1 = 0;
    commandSize2 = 0;
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
            printf("pipeFlag: %d\n", pipeFlag);
            pipeFlag = 1;
        }
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = '\0';
}

/**
 * @brief Function to parse pipe input string into space separated tokens for both commands
 *
 * @param str
 * @param args
 * @param args1
 * @param args2
 */
void inputParserPipe(char *str, char *args[INSIZE], char *args1[INSIZE], char *args2[INSIZE])
{
    printf("commandSize1: %d\n commandSize2: %d\n", commandSize1, commandSize2);
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

int main(int argc, char *argv[])
{

    // get current working directory
    if (getcwd(HOME, sizeof(HOME)) != NULL)
    {
        // printf("Current working dir: %s\n", HOME);
    }
    else
    {
        perror("getcwd() error");
        return 1;
    }

    strcat(HOME1, HOME);
    strcat(HOME1, "/home/user"); // assuming shell is run from the submission directory, adding /home/user to the path
    // printf("PWD: %s\n", HOME1);
    strcpy(PWD, HOME1);
    chdir(HOME1); // changing directory to home directory

    printf("hello (pid:%d)\n", (int)getpid());
    while (1)
    {
        // Taking string input in C - https://stackoverflow.com/a/58703958/23151163
        // chdir(PWD);
        char str[INSIZE];                  // to take user input
        char history[HISTORYSIZE][INSIZE]; // to store history of commands

        printf("MTL458 >");
        scanf(" %99[^\n]", str);

        // storing the command in history
        strcpy(history[COUNT % HISTORYSIZE], str);
        COUNT++;

        // termination on user interrupt
        if (strcmp(str, "exit") == 0)
        {
            exit(0);
        }

        // printing history and continue
        if (strcmp(str, "history") == 0)
        {
            printHistory(history);
            continue;
        }

        // empty command - input not accepting only newline
        // if (strcmp(str, "") == 0)
        // {
        //     continue;
        // }

        pipeFlag = 0; // Need <stdbool.h> for true and false :-(

        // parsing the string into space separated tokens for args
        // https://stackoverflow.com/a/3890186/23151163
        char *args[INSIZE];
        inputParser(str, args);

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
            inputParserPipe(str, args, args1, args2);
        }

        // printf("commandSize1: %d\n commandSize2: %d\n", commandSize1, commandSize2);

        // exit if exit is entered in pipe commands
        // exit brfore fork to avoid killing parent children stuff
        if (pipeFlag && commandSize1 && commandSize2 && (strcmp(args1[0], "exit") == 0 || strcmp(args2[0], "exit") == 0))
        {
            exit(1);
        }

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

            if (execvp(args[0], args) < 0)
            {
                printf("Invalid Command\n");
                exit(1);
            }
            // exit(0);
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

                // handling history command
                if (strcmp(args1[0], "history") == 0)
                {
                    printHistory(history);
                    exit(0);
                }

                if (execvp(args1[0], args1) < 0)
                {
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
                close(STDIN_FILENO); // close normal stdin
                dup(pipefd[0]);      // make stdin come from read end of pipe
                close(pipefd[1]);    // close write end of pipe

                // handling history command
                if (strcmp(args2[0], "history") == 0)
                {
                    printHistory(history);
                    exit(0);
                }
                if (execvp(args2[0], args2) < 0)
                {
                    printf("Invalid Command\n");
                    exit(1);
                }
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
/*
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡔⠒⠤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⣀⠤⠤⠤⠵⣄⠀⠈⠳⣄⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠓⠤⢀⣀⣠⠤⠷⠦⠤⠬⣦⣀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⣠⠖⠋⠁⠀⠀⠀⠀⠀⠀⠀⠈⠙⢦⡀⠀⠀⠀
⠀⠀⠀⠀⠀⣠⠞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢦⠀⠀
⠀⠀⣠⡀⣰⠃⠀⠀⠀⠀⠀⠀⢀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣧⠀
⣿⣮⣿⣷⠃⠀⠀⠀⠀⠀⠀⠀⠸⠿⣿⣷⣶⣤⣠⣤⣶⣾⣿⣿⡇
⠉⢭⡿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠁⠀⣽⡏⣟⣿⡍⠁⠡⠀⣷
⠀⠈⠀⢻⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠢⢀⡠⠚⠉⠑⢤⡔⠁⠀⡇
⠀⠀⠀⠘⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⣎⣀⡀⠀⠀⠀⠙⣄⣰⠃
⠀⠀⠀⠀⠙⢆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢢⡀⠀⢈⠝⠋⣹⠃⠀
⠀⠀⠀⠀⠀⠈⠓⢦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠚⢁⡤⠞⠁⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠒⠦⠤⠤⠤⠤⠴⠖⠚⠉⠀⠀⠀⠀⠀
*/