#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

#define SIZE 1000000
#define MAX_VARS 100
#define MAX_VAR_LEN 50
#define MAX_VAL_LEN 200
char input[SIZE];
char buffer[SIZE];
char cwd[SIZE];
int last_status = 0;        // holds exit status of last command
int var_count = 0;          // number of shell variables stored
int suppress_output = 0;    // when in test mode, silences prompts & errors

// storage for shell variables
struct {
    char name[MAX_VAR_LEN];
    char value[MAX_VAL_LEN];
} variables[MAX_VARS];

int nanoshell_main(int argc, char *argv[]) {


    

    while (1) {
        // print prompt (unless silenced)
        if (!suppress_output) {
            printf("nano shell -> ");
            fflush(stdout);
        }

        // read a line; exit on EOF
        if (!fgets(input, SIZE, stdin))
            break;

        // strip trailing newline
        input[strcspn(input, "\n")] = '\0';

        // trim leading whitespace
        char *p = input;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0')                    // blank line?
            continue;
        memmove(input, p, strlen(p) + 1);

        // check for variable assignment (name=value)
        char *eq = strchr(input, '=');
        if (eq) {
            // validate var name: must start with letter or '_' and contain only [A-Za-z0-9_]
            char *name = input;
            int valid = (isalpha((unsigned char)*name) || *name == '_');
            for (char *q = name; q < eq && valid; q++)
                if (!isalnum((unsigned char)*q) && *q != '_')
                    valid = 0;
            if (valid) {
                *eq = '\0';
                char *val = eq + 1;
                // trim whitespace around name and value
                while (isspace((unsigned char)*name)) name++;
                char *end = name + strlen(name) - 1;
                while (end > name && isspace((unsigned char)*end)) *end-- = '\0';
                while (isspace((unsigned char)*val)) val++;
                end = val + strlen(val) - 1;
                while (end > val && isspace((unsigned char)*end)) *end-- = '\0';

                // store or update variable
                int i;
                for (i = 0; i < var_count; i++) {
                    if (strcmp(variables[i].name, name) == 0) {
                        strncpy(variables[i].value, val, MAX_VAL_LEN);
                        break;
                    }
                }
                if (i == var_count) {
                    if (var_count < MAX_VARS) {
                        strncpy(variables[var_count].name, name,  MAX_VAR_LEN);
                        strncpy(variables[var_count].value, val, MAX_VAL_LEN);
                        var_count++;
                        last_status = 0;
                    } else {
                        if (!suppress_output)
                            fprintf(stderr, "Maximum variables reached\n");
                        last_status = 1;
                    }
                } else {
                    last_status = 0;
                }
                continue;
            }
        }
        else
        {
            char *in = input, *out = buffer;
            while (*in) {
                if (*in == '$') {
                    in++;
                    char varname[MAX_VAR_LEN] = {0}, *v = varname;
                    while (isalnum((unsigned char)*in) || *in == '_')
                        *v++ = *in++;
                    *v = '\0';
                    // search shell vars
                    int found = 0;
                    for (int j = 0; j < var_count; j++) {
                        if (strcmp(variables[j].name, varname) == 0) {
                            strcpy(out, variables[j].value);
                            out += strlen(out);
                            found = 1;
                            break;
                        }
                    }
                    // fallback to env
                    if (!found) {
                        char *env = getenv(varname);
                        if (env) {
                            strcpy(out, env);
                            out += strlen(out);
                        }
                    }
                } else {
                    *out++ = *in++;
                }
            }
            *out = '\0';
            strcpy(input, buffer);
        }

        // dispatch built‑in commands
        if (strncmp(input, "echo", 4) == 0 &&
            (input[4] == '\0' || isspace((unsigned char)input[4]))) {
            // echo: print tokens separated by space
            char *tok = strtok(input + 4, " \t");
            while (tok) {
                printf("%s", tok);
                tok = strtok(NULL, " \t");
                if (tok) printf(" ");
            }
            printf("\n");
            last_status = 0;
        }
        else if (strcmp(input, "exit") == 0) {
            if (!suppress_output) printf("Good Bye\n");
            break;
        }
        else if (strncmp(input, "cd ", 3) == 0) {
            char *dir = input + 3;
            // trim leading/trailing spaces
            while (isspace((unsigned char)*dir)) dir++;
            char *end = dir + strlen(dir) - 1;
            while (end > dir && isspace((unsigned char)*end)) *end-- = '\0';
            if (chdir(dir) == -1) {
                if (!suppress_output)
                    fprintf(stderr, "cd: %s: %s\n", dir, strerror(errno));
                last_status = 1;
            } else {
                last_status = 0;
            }
        }
        else if (strcmp(input, "pwd") == 0) {
            if (getcwd(cwd, sizeof(cwd))) {
                printf("%s\n", cwd);
                last_status = 0;
            } else {
                if (!suppress_output) perror("pwd");
                last_status = 1;
            }
        }
        else if (strncmp(input, "export ", 7) == 0) {
            char *assignment = input + 7;
            char *es = strchr(assignment, '=');
            if (!es || es == assignment || !*(es+1)) {
                if (!suppress_output) fprintf(stderr, "Invalid export syntax\n");
                last_status = 1;
            } else {
                *es = '\0';
                if (setenv(assignment, es+1, 1)) {
                    if (!suppress_output) perror("export");
                    last_status = 1;
                } else {
                    last_status = 0;
                }
            }
        }
        else if (strcmp(input, "printenv") == 0) {
            extern char **environ;
            for (char **e = environ; *e; e++)
                printf("%s\n", *e);
            last_status = 0;
        }
        else if (strcmp(input, "test_mode_on") == 0) {
            suppress_output = 1;
            last_status = 0;
        }
        else if (strcmp(input, "test_mode_off") == 0) {
            suppress_output = 0;
            last_status = 0;
        }
        else {
            // external command: fork/exec
            char *argv_exec[SIZE/2];
            int argc_exec = 0;
            char *tok = strtok(input, " ");
            while (tok && argc_exec < SIZE/2 - 1) {
                argv_exec[argc_exec++] = tok;
                tok = strtok(NULL, " ");
            }
            argv_exec[argc_exec] = NULL;

            pid_t pid = fork();
            if (pid < 0) {
                if (!suppress_output) perror("fork");
                last_status = 1;
            }
            else if (pid == 0) {
                execvp(argv_exec[0], argv_exec);
                if (!suppress_output) {
                    if (errno == ENOENT)
                        fprintf(stderr, "%s: command not found\n", argv_exec[0]);
                    else
                        perror(argv_exec[0]);
                }
                _exit(127);
            }
            else {
                int status;
                waitpid(pid, &status, 0);
                last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
            }
        }
    }

    return last_status;
}
