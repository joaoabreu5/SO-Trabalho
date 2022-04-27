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

char *get_path(char *exec, char *directory)
{
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

void free_args_cliente_array(char **args_cliente, int n_args)
{
    int i;
    for (i = 0; i < n_args; i++)
    {
        if (args_cliente[i] != NULL)
        {
            free(args_cliente[i]);
        }
    }
    if (args_cliente != NULL)
    {
        free(args_cliente);
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
    char *args_copy = strdup(args), *executavel = NULL, *args_copy_free = NULL;
    char **argv = NULL;

    args_copy_free = args_copy;

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

    if (args_copy_free != NULL)
    {
        free(args_copy_free);
    }

    return argv;
}

int check_resources(char **args, int n_args, Operation maxOperations, Operation curOperations)
{
    int i, r = 1;
    int nop, bcompress, bdecompress, gcompress, gdecompress, encrypt, decrypt;
    nop = bcompress = bdecompress = gcompress = gdecompress = encrypt = decrypt = 0;

    for (i = 1; i < n_args && r == 1; i++)
    {
        if (strcmp(args[i], "nop") == 0)
        {
            nop++;
            if (curOperations->nop + nop > maxOperations->nop)
                r = 0;
        }
        else if (strcmp(args[i], "bcompress") == 0)
        {
            bcompress++;
            if (curOperations->bcompress + bcompress > maxOperations->bcompress)
                r = 0;
        }
        else if (strcmp(args[i], "bdecompress") == 0)
        {
            bdecompress++;
            if (curOperations->bdecompress + bdecompress > maxOperations->bdecompress)
                r = 0;
        }
        else if (strcmp(args[i], "gcompress") == 0)
        {
            gcompress++;
            if (curOperations->gcompress + gcompress > maxOperations->gcompress)
                r = 0;
        }
        else if (strcmp(args[i], "gdecompress") == 0)
        {
            gdecompress++;
            if (curOperations->gdecompress + gdecompress > maxOperations->gdecompress)
                r = 0;
        }
        else if (strcmp(args[i], "encrypt") == 0)
        {
            encrypt++;
            if (curOperations->encrypt + encrypt > maxOperations->encrypt)
                r = 0;
        }
        else if (strcmp(args[i], "decrypt") == 0)
        {
            decrypt++;
            if (curOperations->decrypt + decrypt > maxOperations->decrypt)
                r = 0;
        }
    }

    if (r == 1)
    {
        curOperations->nop = curOperations->nop + nop;
        curOperations->bcompress = curOperations->bcompress + bcompress;
        curOperations->bdecompress = curOperations->bdecompress + bdecompress;
        curOperations->gcompress = curOperations->gcompress + gcompress;
        curOperations->gdecompress = curOperations->gdecompress + gdecompress;
        curOperations->encrypt = curOperations->encrypt + encrypt;
        curOperations->decrypt = curOperations->decrypt + decrypt;
    }

    return r;
}

void decrement_resources(char **args, int n_args, Operation curOperations)
{
    int i;
    for (i = 1; i < n_args; i++)
    {
        if (strcmp(args[i], "nop") == 0)
        {
            (curOperations->nop)--;
        }
        else if (strcmp(args[i], "bcompress") == 0)
        {
            (curOperations->bcompress)--;
        }
        else if (strcmp(args[i], "bdecompress") == 0)
        {
            (curOperations->bdecompress)--;
        }
        else if (strcmp(args[i], "gcompress") == 0)
        {
            (curOperations->gcompress)--;
        }
        else if (strcmp(args[i], "gdecompress") == 0)
        {
            (curOperations->gdecompress)--;
        }
        else if (strcmp(args[i], "encrypt") == 0)
        {
            (curOperations->encrypt)--;
        }
        else if (strcmp(args[i], "decrypt") == 0)
        {
            (curOperations->decrypt)--;
        }
    }
}

int proc_file(int argc, char *argv[], char *execs_directory)
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
            path = get_path(argv[3], execs_directory);
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
            comandos[i][0] = get_path(argv[i + 3], execs_directory);
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

            Operation maxOperations = NULL, curOperations = NULL;
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
                    int n_args_cliente = 0;
                    char **args_cliente = parse_args(args, &n_args_cliente);

                    if (read_res > 0)
                    {

                        if (strcmp("proc-file", args_cliente[0]) == 0)
                        {
                            if (check_resources(args_cliente, n_args_cliente, maxOperations, curOperations) == 1)
                            {
                                proc_file(n_args_cliente, args_cliente, argv[2]);
                                decrement_resources(args_cliente, n_args_cliente, curOperations);
                            }
                            print_Op(curOperations);
                        }
                        else if (strcmp("status", args_cliente[0]) == 0)
                        {
                        }
                        else if (strcmp("end", args_cliente[0]) == 0)
                        {
                            read_res = 0;
                        }
                        else
                        {
                            write(2, "Error: Command is not valid.\n", 30);
                        }
                    }

                    free_args_cliente_array(args_cliente, n_args_cliente);
                }
                else
                {
                    write(2, "Error: Command is not valid.\n", 30);
                }

                close(fd_fifo);

            } while (read_res > 0);

            free(maxOperations);
            free(curOperations);

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