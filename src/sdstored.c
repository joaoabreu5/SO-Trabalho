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
#include <signal.h>

#define SIZE 1024

typedef void (*sighandler_t)(int);

int sig_term = 0, fiford, fifowr;

void sigterm_handler(int signum)
{
    sig_term = 1;
    close(fifowr);
    write(1, "SIGTERM received... Waiting for queue to end\n", 46);
}

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

void status(List *queue, Operation maxOperations, Operation curOperations, int fd_client_fifo)
{
    char aux[1024];
    while (queue != NULL)
    {
        if (queue->commands.type == 0)
        {
            snprintf(aux, sizeof(aux), "task #%d: proc-file -p %d ", queue->commands.task_number, queue->commands.priority);
            strcat(aux, queue->commands.commands);
            write(fd_client_fifo, &aux, sizeof(aux));
        }
        queue = queue->next;
    }
    snprintf(aux, sizeof(aux), "transf nop: %d/%d (running/max)", curOperations->nop, maxOperations->nop);
    write(fd_client_fifo, &aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "transf bcompress: %d/%d (running/max)", curOperations->bcompress, maxOperations->bcompress);
    write(fd_client_fifo, &aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "transf bdecompress: %d/%d (running/max)", curOperations->bdecompress, maxOperations->bdecompress);
    write(fd_client_fifo, &aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "transf gcompress: %d/%d (running/max)", curOperations->gcompress, maxOperations->gcompress);
    write(fd_client_fifo, &aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "transf gdecompress: %d/%d (running/max)", curOperations->gdecompress, maxOperations->gdecompress);
    write(fd_client_fifo, &aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "transf encrypt: %d/%d (running/max)", curOperations->encrypt, maxOperations->encrypt);
    write(fd_client_fifo, &aux, sizeof(aux));

    snprintf(aux, sizeof(aux), "transf decrypt: %d/%d (running/max)", curOperations->decrypt, maxOperations->decrypt);
    write(fd_client_fifo, &aux, sizeof(aux));

    write(fd_client_fifo, "concluded", 10);
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

char *concluded_string(int input_bytes, int output_bytes)
{
    int str_size = 42 + number_of_Digits(input_bytes) + number_of_Digits(output_bytes);
    char *buffer = calloc(str_size, sizeof(char));
    snprintf(buffer, str_size, "concluded (bytes-input: %d, bytes-output: %d)", input_bytes, output_bytes);
    return buffer;
}

int proc_file(int argc, char *argv[], char *execs_directory, int fd_client_fifo)
{
    int command_number = argc - 2;
    int i, j, r_exec, r_pipe, pipes[command_number - 1][2], input_bytes, output_bytes;
    char *path = NULL, ***comandos = NULL;

    write(fd_client_fifo, "processing", 11);

    create_directory(argv[1]);

    int fd_in = open(argv[0], O_RDONLY);
    int fd_out = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    int fd_0 = dup(0);
    int fd_1 = dup(1);

    dup2(fd_in, 0);
    dup2(fd_out, 1);

    input_bytes = lseek(fd_in, 0, SEEK_END);
    lseek(fd_in, 0, SEEK_SET);

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
                write(2, "Error: Creating pipe\n", 22);
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

    fd_out = open(argv[1], O_RDONLY, 0666);
    output_bytes = lseek(fd_out, 0, SEEK_END);
    close(fd_out);

    char *str_concluded = concluded_string(input_bytes, output_bytes);
    write(fd_client_fifo, str_concluded, strlen(str_concluded) + 1);
    free(str_concluded);

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
    signal(SIGTERM, sigterm_handler);
    if (argc < 3)
    {
        write(2, "Error: Not enough arguments\n", 29);
        return -1;
    }

    int fd_client_fifo;
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
            int p[2], spid;

            if (pipe(p) == -1)
            {
                write(2, "Error: Creating pipe\n", 22);
                return -1;
            }

            if ((spid = fork()) == 0)
            {
                Node *queue = NULL;
                List *executing_queue = NULL;
                int hasCommands;
                message buf, exec;
                int n_read;
                char client_fifo[1024];
                while (1)
                {
                    if (sig_term == 1 && isEmpty(&queue))
                    {
                        close(p[1]);
                    }
                    hasCommands = 1;
                    n_read = read(p[0], &buf, sizeof(message));
                    if (n_read > 0)
                    {
                        if (buf.type == -1)
                        {
                            sig_term = 1;
                        }
                        else
                        {
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
                            }

                            while (!isEmpty(&queue) && hasCommands)
                            {
                                exec = peek(&queue);
                                if (exec.type == 0)
                                {
                                    if (check_resources(&exec.op, maxOperations, curOperations))
                                    {
                                        if (fork() == 0)
                                        {
                                            close(p[0]);
                                            snprintf(client_fifo, 1024, CLIENT_FIFO_NAME, (int)exec.client_pid);

                                            char **args_cliente;

                                            if ((fd_client_fifo = open(client_fifo, O_WRONLY)) == -1)
                                            {
                                                write(2, "Error: Opening client FIFO\n", 28);
                                                return -1;
                                            }
                                            args_cliente = parse_args(exec.commands, exec.n_args);
                                            proc_file(exec.n_args, args_cliente, argv[2], fd_client_fifo);
                                            free_args_cliente_array(args_cliente, exec.n_args);

                                            write(p[1], &exec, sizeof(message));
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

                                        if ((fd_client_fifo = open(client_fifo, O_WRONLY)) == -1)
                                        {
                                            write(2, "Error: Opening client FIFO\n", 28);
                                            return -1;
                                        }
                                            
                                        status(executing_queue, maxOperations, curOperations, fd_client_fifo);

                                        write(p[1], &exec, sizeof(message));
                                        close(p[1]);
                                        close(fd_client_fifo);
                                        _exit(0);
                                    }
                                    add_elem(&executing_queue, exec);
                                }
                            }
                        }
                    }
                    else
                    {
                        close(p[0]);
                        return 0;
                    }
                }
            }

            close(p[0]);

            mkfifo(SERVER_FIFO_NAME, 0777);

            int read_res;
            char client_fifo[1024];

            fiford = open(SERVER_FIFO_NAME, O_RDONLY);
            fifowr = open(SERVER_FIFO_NAME, O_WRONLY);

            message messageFromClient;

            int task_n = 0;

            while ((read_res = read(fiford, &messageFromClient, sizeof(message))) > 0)
            {
                snprintf(client_fifo, 1024, CLIENT_FIFO_NAME, (int)messageFromClient.client_pid);

                if ((fd_client_fifo = open(client_fifo, O_WRONLY)) == -1)
                {
                    write(2, "Error: Opening client FIFO\n", 28);
                    return -1;
                }

                if (isValid(&messageFromClient.op, maxOperations) && !sig_term)
                {
                    task_n++;
                    messageFromClient.task_number = task_n;
                    write(fd_client_fifo, "pending", 8);
                    close(fd_client_fifo);
                    write(p[1], &messageFromClient, sizeof(message));
                }
                else
                {
                    write(fd_client_fifo, "concluded", 10);
                    close(fd_client_fifo);
                }
            }
            close(fiford);
            unlink(SERVER_FIFO_NAME);
            messageFromClient.type = -1;
            write(p[1], &messageFromClient, sizeof(message));
            close(p[1]);
            wait(NULL);
        }
        else
        {
            write(2, "Error: Directory does not exist\n", 33);
            return -1;
        }
    }
    else
    {
        write(2, "Error: Configuration file does not exist\n", 42);
        close(configFile);
        return -1;
    }
    return 0;
}