#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    int fdfifo = open("fifo", O_WRONLY), i;
    if (argc > 1)
    {
        int arguments = argc - 1;
        char args[1000];
        if (arguments > 1)
        {
            strcpy(args, argv[1]);
            strcat(args, " ");
            for (i = 2; i < argc - 1; i++)
            {
                strcat(args, argv[i]);
                strcat(args, " ");
            }
            strcat(args, argv[i]);
            strcat(args, "\0");
        }
        else
        {
            strcpy(args, argv[1]);
            strcat(args, "\0");
        }
        write(fdfifo, args, sizeof(args));
    }
    close(fdfifo);
}