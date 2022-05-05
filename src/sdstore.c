#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "declarations.h"

int number_of_Digits(int x)
{
    int count = 0;
    if (x < 0)
    {
        count++;
    }
    do
    {
        x /= 10;
        count++;
    } while (x != 0);
    return count;
}

int has_priority(char *arg)
{
    int r = 0;
    if(strcmp(arg, "0") == 0 || strcmp(arg, "1") == 0 || strcmp(arg, "2") == 0 || strcmp(arg, "3") == 0 || strcmp(arg, "4") == 0 || strcmp(arg, "5") == 0)
    {
        r = 1;
    }
    return r;
}

int main(int argc, char *argv[])
{
    int fd_srv_fifo = open(SERVER_FIFO_NAME, O_WRONLY);
    int i, args_size = 0;

    if (fd_srv_fifo > 0)
    {
        if (argc >= 2)
        {
            if (strcmp(argv[1], "status") == 0 || strcmp(argv[1], "proc-file") == 0 || strcmp(argv[1], "exit") == 0)
            {
                message st_message;
                st_message.client_pid = getpid();
                char cli_fifo[1024], buf[1024];
                snprintf(cli_fifo, sizeof(cli_fifo), CLIENT_FIFO_NAME, (int)st_message.client_pid);
                mkfifo(cli_fifo, 0777);
                int fd_clififowr, fd_clififord, bytes_read, first_index = 0;
                if (argc > 2)
                {
                    if (strcmp(argv[1], "proc-file") != 0)
                    {
                        write(2, "Error: Command not valid\n", 26);
                        unlink(cli_fifo);
                        _exit(EXIT_FAILURE);
                    }
                    st_message.type = 0;

                    int arguments = 0;

                    if (strcmp(argv[2], "-p") == 0 && has_priority(argv[3]) == 1)
                    {
                        arguments = argc - 4;
                        first_index = 4;
                        st_message.priority = atoi(argv[3]);
                    }
                    else
                    {
                        arguments = argc-2;
                        first_index = 2;
                        st_message.priority = 0;
                    }

                    st_message.n_args = arguments;
                    int n_args_len = number_of_Digits(arguments) + 1;

                    for (i = 2; i < argc; i++)
                    {
                        args_size += strlen(argv[i]);
                    }
                    args_size += n_args_len + arguments;

                    char *args = calloc(1024, sizeof(char));
                    char *n_args = malloc(n_args_len * sizeof(char));

                    snprintf(n_args, n_args_len, "%d", arguments);

                    if (arguments > 0)
                    {
                        for (i = first_index; i < argc-1; i++)
                        {
                            strcat(args, argv[i]);
                            strcat(args, " ");
                        }
                        strcat(args, argv[i]);
                    }
                    strcat(args, "\0");

                    strcpy(st_message.commands, args);
                    free(n_args);
                }
                else if (argc == 2)
                {
                    if (strcmp(argv[1], "proc-file") == 0)
                    {
                        close(fd_srv_fifo);
                        write(2, "Error: Command not valid\n", 26);
                        unlink(cli_fifo);
                        _exit(0);
                    }
                    else
                    {
                        st_message.type = strcmp("status", argv[1]) == 0 ? 1 : 2;
                        strcpy(st_message.commands, argv[1]);
                    }
                }
                
                write(fd_srv_fifo, &st_message, sizeof(st_message));
                close(fd_srv_fifo);

                fd_clififord = open(cli_fifo, O_RDONLY);
                fd_clififowr = open(cli_fifo, O_WRONLY);
                while ((bytes_read = read(fd_clififord, buf, sizeof(buf))) > 0)
                {
                    printf("%s\n", buf);
                    if (strcmp(buf, "Terminated") == 0)
                        break;
                }
                close(fd_clififord);
                close(fd_clififowr);
                unlink(cli_fifo);
            }
            else
            {
                write(2, "Error: Command not valid\n", 26);
            }
        }
        else
        {
            write(2, "Error: Command not valid\n", 26);
        }
    }
    else
    {
        write(2, "Error: Fifo does not exist.\n", 29);
        close(fd_srv_fifo);
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}