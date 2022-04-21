#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SIZE 1024

char *get_path(char *exec)
{
    char *directory = "./SDStore/";
    int path_size = strlen(directory)+strlen(exec)+1;
    char *path = calloc(path_size, sizeof(char));
    strcat(path, directory);
    strcat(path, exec);
    return path;
}

void free_command_array(char ***comandos, int N_linhas, int N_colunas)
{
    int i, j;
    for (i=0; i<N_linhas; i++)
    {
        for (j=0; j<N_colunas; j++)
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

int proc_file(int argc, char *argv[])
{
    int command_number = argc - 4;
    int i, j, r_exec, r_pipe, pipes[command_number-1][2];
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
            path = get_path(argv[4]);
            r_exec = execl(path, argv[4], NULL);
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
        for(i=0; i<command_number; i++)
        {
            comandos[i] = malloc(3 * sizeof(char **));
            comandos[i][0] = get_path(argv[i+4]);
            comandos[i][1] = strdup(argv[i+4]);
            comandos[i][2] = NULL;
        }

        for(i=0; i<command_number-1; i++)
        {
            r_pipe = pipe(pipes[i]);
            if (r_pipe == -1)
            {
                perror("pipe");
                return -1;
            }
        }

        if (fork() == 0)
        {
            close(pipes[0][0]);
            for (i=1; i<command_number-1; i++)
            {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }
            dup2(pipes[0][1], 1);
            close(pipes[0][1]);
            r_exec = execvp(comandos[0][0], comandos[0]);
            _exit(r_exec);
        }
        close(pipes[0][1]);

        for(i=1; i<command_number-1; i++)
        {
            if (fork() == 0)
            {
                close(pipes[i][0]);
                for (j=i+1; j<command_number-1; j++)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                dup2(pipes[i-1][0], 0);
                dup2(pipes[i][1], 1);
                close(pipes[i-1][0]);
                close(pipes[i][1]);
                r_exec = execvp(comandos[i][0], comandos[i]);
                _exit(r_exec);
            }
            close(pipes[i - 1][0]);
            close(pipes[i][1]);
        }

        if (fork() == 0)
        {
            dup2(pipes[i-1][0], 0);
            close(pipes[i-1][0]);
            r_exec = execvp(comandos[i][0], comandos[i]);
            _exit(r_exec);
        }
        close(pipes[i-1][0]);

        for(i=0; i<command_number; i++)
        {
            wait(NULL);
        }

        free_command_array(comandos, command_number, 2);
    }

    dup2(fd_0, 0);
    dup2(fd_1, 1);

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "proc-file") == 0)
    {
        proc_file(argc, argv);
    }
    return 0;
}
