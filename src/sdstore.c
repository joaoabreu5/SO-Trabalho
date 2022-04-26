#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

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
    int fd_fifo = open("fifo", O_WRONLY);
    int i, args_size = 0;

    if (fd_fifo > 0)
    {
        if (argc > 1)
        {
            int arguments = argc - 1;
            int n_args_len = number_of_Digits(arguments) + 1;

            for(i=1; i<argc; i++)
            {
                args_size += strlen(argv[i]);
            }
            args_size += n_args_len + arguments;

            char *args = malloc(args_size * sizeof(char));
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

            write(fd_fifo, args, strlen(args) + 1);
            close(fd_fifo);
            
            free(args);
            free(n_args);
        }
        else
        {
            write(fd_fifo, "Error", 6);
            write(2, "Error: Not enough arguments.\n", 30);
            close(fd_fifo);
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        write(2, "Error: Fifo does not exist.\n", 29);
        close(fd_fifo);
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}