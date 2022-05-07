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
#include "queue.h"

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

char **parse_args(char *args, int n_args)
{
    int i = 0;
    char *args_copy = strdup(args), *executavel = NULL, *args_copy_free = NULL;
    char **argv = NULL;

    args_copy_free = args_copy;

    argv = malloc(n_args * sizeof(char *));
    while ((executavel = strsep(&args_copy, " ")) != NULL && i < n_args)
    {
        argv[i] = strdup(executavel);
        i++;
    }

    if (args_copy_free != NULL)
    {
        free(args_copy_free);
    }

    return argv;
}

void status(Node *queue, Operation maxOperations, Operation curOperations, int fd_client_fifo)
{
    char aux[1024];
    while (queue != NULL)
    {
        snprintf(aux, sizeof(aux), "Priority: %d Task: proc-file ", queue->commands.priority);
        strcat(aux, queue->commands.commands);
        write(fd_client_fifo, aux, sizeof(aux));
        queue = queue->next;
    }
    snprintf(aux, sizeof(aux), "Nop: %d/%d", curOperations->nop, maxOperations->nop);
    write(fd_client_fifo, aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "Bcompress: %d/%d", curOperations->bcompress, maxOperations->bcompress);
    write(fd_client_fifo, aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "Bdecompress: %d/%d", curOperations->bdecompress, maxOperations->bdecompress);
    write(fd_client_fifo, aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "Gcompress: %d/%d", curOperations->gcompress, maxOperations->gcompress);
    write(fd_client_fifo, aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "Gdecompress: %d/%d", curOperations->gdecompress, maxOperations->gdecompress);
    write(fd_client_fifo, aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "Encrypt: %d/%d", curOperations->encrypt, maxOperations->encrypt);
    write(fd_client_fifo, aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "Decrypt: %d/%d", curOperations->decrypt, maxOperations->decrypt);
    write(fd_client_fifo, aux, sizeof(aux));

    write(fd_client_fifo, "Terminated", 11);
}

int check_resources(Operation lastOp, Operation maxOperations, Operation curOperations)
{
    int r = 1;

    if (curOperations->nop + lastOp->nop > maxOperations->nop)
        r = 0;
    else if (curOperations->bcompress + lastOp->bcompress > maxOperations->bcompress)
        r = 0;
    else if (curOperations->bdecompress + lastOp->bdecompress > maxOperations->bdecompress)
        r = 0;
    else if (curOperations->gcompress + lastOp->gcompress > maxOperations->gcompress)
        r = 0;
    else if (curOperations->gdecompress + lastOp->gdecompress > maxOperations->gdecompress)
        r = 0;
    else if (curOperations->encrypt + lastOp->encrypt > maxOperations->encrypt)
        r = 0;
    else if (curOperations->decrypt + lastOp->decrypt > maxOperations->decrypt)
        r = 0;

    if (r == 1)
    {
        curOperations->nop = curOperations->nop + lastOp->nop;
        curOperations->bcompress = curOperations->bcompress + lastOp->bcompress;
        curOperations->bdecompress = curOperations->bdecompress + lastOp->bdecompress;
        curOperations->gcompress = curOperations->gcompress + lastOp->gcompress;
        curOperations->gdecompress = curOperations->gdecompress + lastOp->gdecompress;
        curOperations->encrypt = curOperations->encrypt + lastOp->encrypt;
        curOperations->decrypt = curOperations->decrypt + lastOp->decrypt;
    }

    return r;
}

void decrement_resources(Operation lastOp, Operation curOperations)
{
    curOperations->bcompress -= lastOp->bcompress;
    curOperations->bdecompress -= lastOp->bdecompress;
    curOperations->decrypt -= lastOp->decrypt;
    curOperations->encrypt -= lastOp->encrypt;
    curOperations->gcompress -= lastOp->gcompress;
    curOperations->gdecompress -= lastOp->gdecompress;
    curOperations->nop -= lastOp->nop;
}

int proc_file(int argc, char *argv[], char *execs_directory, int fd_client_fifo)
{
    int command_number = argc - 2;
    int i, j, r_exec, r_pipe, pipes[command_number - 1][2];
    char *path = NULL, ***comandos = NULL;

    write(fd_client_fifo, "Processing", 12);

    create_directory(argv[1]);

    int fd_in = open(argv[0], O_RDONLY);
    int fd_out = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);

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
            path = get_path(argv[2], execs_directory);
            r_exec = execl(path, argv[2], NULL);
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
            comandos[i][0] = get_path(argv[i + 2], execs_directory);
            comandos[i][1] = strdup(argv[i + 2]);
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
    write(fd_client_fifo, "Terminated", 12);
    dup2(fd_0, 0);
    dup2(fd_1, 1);

    return 0;
}

int isValid(Operation lastOp, Operation maxOperations)
{
    int r = 1;
    if (lastOp->nop > maxOperations->nop)
        r = 0;
    else if (lastOp->bcompress > maxOperations->bcompress)
        r = 0;
    else if (lastOp->bdecompress > maxOperations->bdecompress)
        r = 0;
    else if (lastOp->gcompress > maxOperations->gcompress)
        r = 0;
    else if (lastOp->gdecompress > maxOperations->gdecompress)
        r = 0;
    else if (lastOp->encrypt > maxOperations->encrypt)
        r = 0;
    else if (lastOp->decrypt > maxOperations->decrypt)
        r = 0;
    return r;
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
            int p[2];

            if (pipe(p) < 0)
            {
                _exit(1);
            }

            if (fork() == 0)
            {
                Node *queue = NULL;
                List *executing_queue = NULL;
                int i = 0, hasCommands;
                message buf, exec;
                int n_read, fd_client_fifo;
                char client_fifo[1024];
                while (1)
                {
                    hasCommands = 1;
                    n_read = read(p[0], &buf, sizeof(buf));
                    if (contains(executing_queue, buf.client_pid))
                    {
                        switch (buf.type)
                        {
                        case 0:
                            decrement_resources(&buf.op, curOperations);
                            break;
                        default:
                            break;
                        }
                        remove_elem(&executing_queue, buf.client_pid);
                        printf("Taking from executing queue\n");
                    }
                    else
                    {
                        if (isEmpty(&queue))
                        {
                            queue = newNode(buf);
                        }
                        else
                        {
                            push(&queue, buf);
                        }
                        printf("Adding to queue\n");
                    }

                    while (!isEmpty(&queue) && hasCommands)
                    {
                        printf("Queue not empty\n");
                        exec = peek(&queue);
                        printf("First to execute - %s\n", exec.commands);
                        print_Op(&exec.op);
                        if (exec.type == 0)
                        {
                            if (check_resources(&exec.op, maxOperations, curOperations))
                            {
                                printf("Resources checked\n");
                                if (fork() == 0)
                                {
                                    close(p[0]);
                                    snprintf(client_fifo, 1024, CLIENT_FIFO_NAME, (int)exec.client_pid);

                                    int fd_client_fifo;
                                    char **args_cliente;

                                    if ((fd_client_fifo = open(client_fifo, O_WRONLY)) == -1)
                                        perror("open");

                                    args_cliente = parse_args(exec.commands, exec.n_args);
                                    proc_file(exec.n_args, args_cliente, argv[2], fd_client_fifo);
                                    free_args_cliente_array(args_cliente, exec.n_args);

                                    write(p[1], &exec, sizeof(exec));
                                    close(p[1]);
                                    close(fd_client_fifo);
                                    _exit(0);
                                }
                                add_elem(&executing_queue, exec);
                                pop(&queue);
                            }
                            else
                            {
                                hasCommands = 0;
                            }
                        }
                        else
                        {
                            pop(&queue);
                            if (fork() == 0)
                            {
                                close(p[0]);
                                snprintf(client_fifo, 1024, CLIENT_FIFO_NAME, (int)exec.client_pid);

                                int fd_client_fifo;
                                char **args_cliente;

                                if ((fd_client_fifo = open(client_fifo, O_WRONLY)) == -1)
                                    perror("open");

                                status(queue, maxOperations, curOperations, fd_client_fifo);

                                write(p[1], &exec, sizeof(exec));
                                close(p[1]);
                                close(fd_client_fifo);
                                _exit(0);
                            }
                            add_elem(&executing_queue, exec);
                        }
                    }
                }
            }

            close(p[0]);

            mkfifo(SERVER_FIFO_NAME, 0777);

            int read_res;
            char client_fifo[1024];

            int fiford = open(SERVER_FIFO_NAME, O_RDONLY);
            int fifowr = open(SERVER_FIFO_NAME, O_WRONLY);

            message messageFromClient;

            while ((read_res = read(fiford, &messageFromClient, sizeof(messageFromClient))) > 0)
            {
                if (isValid(&messageFromClient.op, maxOperations))
                {
                    write(p[1], &messageFromClient, sizeof(messageFromClient));
                }
                else
                {
                    snprintf(client_fifo, 1024, CLIENT_FIFO_NAME, (int)messageFromClient.client_pid);
                    int fd_client_fifo;

                    if ((fd_client_fifo = open(client_fifo, O_WRONLY)) == -1)
                        perror("open");

                    write(fd_client_fifo, "Terminated", 11);
                    close(fd_client_fifo);
                }
                printf("While reader -> %s\n", messageFromClient.commands);
            }
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