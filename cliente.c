#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SIZE 1024

char *get_path(char *exec)
{
    char *path = calloc(SIZE, sizeof(char));
    strcat(path, "./SDStore/");
    strcat(path, exec);
    return path;
}

void close_npipes(int n, int pipe_array[n][2])
{
    for (int i = 0; i < n; i++)
    {
        close(pipe_array[i][0]);
        close(pipe_array[i][1]);
    }
}

void free_command_array(char ***comandos, int N)
{
    int i, j;
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (comandos[i][j] != NULL)
            {
                free(comandos[i][j]);
            }
        }
        if (comandos[i] != NULL)
        {
            free(comandos[i]);
        }
    }
    if (comandos != NULL)
    {
        free(comandos);
    }
}

void proc_file(int argc, char *argv[])
{
    int command_number = argc - 4;
    int i, j, t, aux, pid, r_exec, pipes[command_number - 1][2];
    char *path = NULL, ***comandos = NULL;

    int fd_in = open(argv[2], O_RDONLY);
    int fd_out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    int fd_0 = dup(0);
    int fd_1 = dup(1);

    dup2(fd_in, 0);
    dup2(fd_out, 1);

    close(fd_in);
    close(fd_out);

    if (command_number == 1)
    {
        if (fork() == 0)
        {
            i = 4;
            path = get_path(argv[i]);
            r_exec = execl(path, argv[i], NULL);
            free(path);
            _exit(r_exec);
        }
        else
        {
            wait(NULL);
        }
    }
    else if (command_number > 1)
    {
        comandos = malloc(command_number * sizeof(char **));
        for (i = 0; i < command_number; i++)
        {
            comandos[i] = malloc(3 * sizeof(char **));
            comandos[i][0] = get_path(argv[i + 4]);
            comandos[i][1] = strdup(argv[i + 4]);
            comandos[i][2] = NULL;
        }

        for (t = 0; t < command_number - 1; t++)
        {
            pipe(pipes[t]);
        }

        if ((pid = fork()) == 0)
        {
            close(pipes[0][0]);
            for (aux = 1; aux < command_number - 1; aux++)
            {
                close(pipes[aux][0]);
                close(pipes[aux][1]);
            }
            dup2(pipes[0][1], 1);
            close(pipes[0][1]);
            execvp(comandos[0][0], comandos[0]);
        }

        close(pipes[0][1]);

        for (i = 1; i < command_number - 1; i++)
        {
            if ((pid = fork()) == 0)
            {
                close(pipes[i][0]);
                for (aux = i + 1; aux < command_number - 1; aux++)
                {
                    close(pipes[aux][0]);
                    close(pipes[aux][1]);
                }
                dup2(pipes[i - 1][0], 0);
                dup2(pipes[i][1], 1);
                close(pipes[i - 1][0]);
                close(pipes[i][1]);
                r_exec = execvp(comandos[i][0], comandos[i]);
            }
            close(pipes[i - 1][0]);
            close(pipes[i][1]);
        }
        if ((pid = fork()) == 0)
        {
            dup2(pipes[i - 1][0], 0);
            close(pipes[i - 1][0]);
            r_exec = execvp(comandos[i][0], comandos[i]);
        }

        close(pipes[i - 1][0]);

        for (j = 0; j < command_number; j++)
        {
            wait(NULL);
        }

        free_command_array(comandos, command_number);
    }

    dup2(fd_0, 0);
    dup2(fd_1, 1);
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "proc-file") == 0)
    {
        proc_file(argc, argv);
    }
    return 0;
}
