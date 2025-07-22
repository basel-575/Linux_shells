#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


#define MAX_INPUT   1024
#define MAX_TOKENS   105


int picoshell_main(int argc, char *argv[]) {


    char input[MAX_INPUT];
    char *args[MAX_TOKENS];
    char cwd[MAX_INPUT];
    ssize_t len;
    int last = 0;

    while (1) {
        // prompt
        printf("PicoShell > ");
        fflush(stdout);
        if (!fgets(input, sizeof(input), stdin)) {
            //printf("\nGoodbye!\n");
            break;
        }

        // tokenize input
        int argc2 = 0;
        char *tok = strtok(input, " \t\n");
        while (tok && argc2 < MAX_TOKENS - 1) {
            args[argc2++] = tok;
            tok = strtok(NULL, " \t\n");
        }
        args[argc2] = NULL;
        if (argc2 == 0) continue;

        // builtins
        if (strcmp(args[0], "exit") == 0) {
            printf("Good Bye\n");
            break;
        }

           if (strcmp(args[0], "pwd") == 0) {
            if (getcwd(cwd, sizeof(cwd)))
            {
                puts(cwd);
                last=0;
            }
            else
            {
                perror("pwd");
                last=1;
            }
            continue;
        }
        if (strcmp(args[0], "cd") == 0) {
            if (argc2 < 2)
            {
                printf("cd: missing argument\n");
                last=1;
            }
            else if (chdir(args[1]) < 0)
            {
                printf("cd: /invalid_directory: No such file or directory\n");
                last=1;
            }
            else last=0;
            continue;
        }
        if (strcmp(args[0], "echo") == 0) {
            for (int i = 1; i < argc2; i++)
                printf("%s%s", args[i], i+1<argc2?" ":"");
            putchar('\n');
            last=0;
            continue;
        }

        // external command
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            printf("%s: command not found\n",args[0]);
            _exit(-1);
        } else if (pid > 0) {
            int wstat;
            waitpid(pid, &wstat, 0);
            if (WIFEXITED(wstat))
            {
                last = WEXITSTATUS(wstat);
                if(last!=0)
                {
                    printf("%s: command not found\n",args[0]);
                }
            }
            else
            {
                last = 1;
                printf("%s: command not found\n",args[0]);
            }
        }
        else {
            perror("fork");
            last=1;
        }
    }

    return last;
}


