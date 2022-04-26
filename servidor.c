#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "parser.h"

#define SIZE 1024

char *get_path(char *exec)
{
    char *directory = "./SDStore-transf/";
    int path_size = strlen(directory) + strlen(exec) + 1;
    char *path = calloc(path_size, sizeof(char));
    strcat(path, directory);
    strcat(path, exec);
    return path;
}

void free_command_array(char ***comandos, int N_linhas, int N_colunas)
{
    int i, j;
    for (i = 0; i < N_linhas; i++)
    {
        for (j = 0; j < N_colunas; j++)
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

void create_directory(char *path)
{
    int i;
    char *path_copy = strdup(path);

    for (i = strlen(path_copy) - 1; i >= 0 && path_copy[i] != '/'; i--)
    {
        path_copy[i] = '\0';
    }

    if (strcmp(path_copy, "./") != 0 && strcmp(path_copy, "\0") != 0)
    {
        mkdir(path_copy, 0777);
    }
    free(path_copy);
}

char **parse_args(char *args, int *n_args)
{
    int i = 0;
    char *args_copy = strdup(args), *executavel = NULL;
    char **argv = NULL;

    if ((executavel = strsep(&args_copy, " ")) != NULL)
    {
        *n_args = atoi(executavel);
        argv = malloc((*n_args) * sizeof(char *));
        while ((executavel = strsep(&args_copy, " ")) != NULL && i < (*n_args))
        {
            argv[i] = strdup(executavel);
            i++;
        }
    }
    return argv;
}

int proc_file(int argc, char *argv[])
{
    int command_number = argc - 3;
    int i, j, r_exec, r_pipe, pipes[command_number - 1][2];
    char *path = NULL, ***comandos = NULL;

    create_directory(argv[2]);

    int fd_in = open(argv[1], O_RDONLY);
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);

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
            path = get_path(argv[3]);
            r_exec = execl(path, argv[3], NULL);
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
            comandos[i][0] = get_path(argv[i + 3]);
            comandos[i][1] = strdup(argv[i + 3]);
            comandos[i][2] = NULL;
        }

        for (i = 0; i < command_number - 1; i++)
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
            for (i = 1; i < command_number - 1; i++)
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

        for (i = 1; i < command_number - 1; i++)
        {
            if (fork() == 0)
            {
                close(pipes[i][0]);
                for (j = i + 1; j < command_number - 1; j++)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                dup2(pipes[i - 1][0], 0);
                dup2(pipes[i][1], 1);
                close(pipes[i - 1][0]);
                close(pipes[i][1]);
                r_exec = execvp(comandos[i][0], comandos[i]);
                _exit(r_exec);
            }
            close(pipes[i - 1][0]);
            close(pipes[i][1]);
        }

        if (fork() == 0)
        {
            dup2(pipes[i - 1][0], 0);
            close(pipes[i - 1][0]);
            r_exec = execvp(comandos[i][0], comandos[i]);
            _exit(r_exec);
        }
        close(pipes[i - 1][0]);

        for (i = 0; i < command_number; i++)
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
    int i;
    if (argc < 3)
    {
        write(2, "Error: Not enough arguments.\n", 30);
        _exit(EXIT_FAILURE);
    }
    int configFile = open(argv[1], O_RDONLY);
    if (configFile > 0)
    {
        DIR *opDir;
        if ((opDir = opendir(argv[2])) != NULL)
        {
            closedir(opDir);

            Operation maxOperations, curOperations;
            maxOperations = parse(configFile);
            curOperations = calloc(1, sizeof(operation));

            mkfifo("fifo", 0777);

            char args[1000];
            int fd_fifo, read_res;

            do
            {
                fd_fifo = open("fifo", O_RDONLY);

                read_res = read(fd_fifo, args, sizeof(args));

                if (strcmp("Error", args) != 0)
                {
                    int n_args = 0;
                    char **argv2 = parse_args(args, &n_args);

                    if (read_res > 0)
                    {

                        if (strcmp("proc-file", argv2[0]) == 0)
                        {
                            proc_file(n_args, argv2);
                        }
                        else if (strcmp("status", argv2[0]) == 0)
                        {
                        }
                        else if (strcmp("end", argv2[0]) == 0)
                        {
                            read_res = 0;
                        }
                        else
                        {
                            write(2, "Error: Command is not valid.\n", 30);
                        }
                    }

                    close(fd_fifo);
                }
                else
                {
                    write(2, "Error: Command is not valid.\n", 30);
                }
            } while (read_res > 0);

            unlink("fifo");
        }
        else
        {
            write(2, "Error: Directory does not exist.\n", 34);
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        write(2, "Error: The file does not exist.\n", 33);
        close(configFile);
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}