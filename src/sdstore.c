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

int main(int argc, char *argv[])
{
    int fd_srv_fifo = open(SERVER_FIFO_NAME, O_WRONLY);
    int i, args_size = 0;

    if (fd_srv_fifo > 0)
    {
        message st_message;
        st_message.client_pid = getpid();
        char cli_fifo[1024], buf[1024];
        snprintf(cli_fifo, sizeof(cli_fifo), CLIENT_FIFO_NAME, (int)st_message.client_pid);
        mkfifo(cli_fifo, 0777);
        int fd_clififowr, fd_clififord, bytes_read;
        if (argc > 1)
        {
            int arguments = argc - 1;
            int n_args_len = number_of_Digits(arguments) + 1;

            for (i = 1; i < argc; i++)
            {
                args_size += strlen(argv[i]);
            }
            args_size += n_args_len + arguments;

            char args[1024];
            char *n_args = malloc(n_args_len * sizeof(char));

            snprintf(n_args, n_args_len, "%d", arguments);

            strcpy(args, n_args);
            strcat(args, " ");

            if (arguments > 1)
            {
                strcat(args, argv[1]);
                strcat(args, " ");
                for (i = 2; i < argc - 1; i++)
                {
                    strcat(args, argv[i]);
                    strcat(args, " ");
                }
                strcat(args, argv[i]);
            }
            else
            {
                strcat(args, argv[1]);
            }
            strcat(args, "\0");

            strcpy(st_message.commands, args);
            free(n_args);
        }
        else
        {
            char args[] = "Error";
            strcpy(st_message.commands, args);
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
        write(2, "Error: Fifo does not exist.\n", 29);
        close(fd_srv_fifo);
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}