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
    int fdfifo = open("fifo", O_WRONLY), i;
    if (fdfifo > 0)
    {
        if (argc > 1)
        {
            int arguments = argc - 1;
            char *args = malloc(1000 * sizeof(char));

            int n_args_len = number_of_Digits(arguments) + 1;

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

            write(fdfifo, args, strlen(args) + 1);
            close(fdfifo);
        }
        else
        {
            write(fdfifo, "Error", 6);
            write(2, "Error: Not enough arguments.\n", 30);
            close(fdfifo);
            _exit(EXIT_FAILURE);
        }
    }
    else
    {
        write(2, "Error: Fifo does not exist.\n", 29);
        close(fdfifo);
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}