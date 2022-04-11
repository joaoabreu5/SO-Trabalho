#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define SIZE 1024 

char *get_path(char *exec)
{
    char *new_exec = strdup(exec);
    char *path = calloc(SIZE, sizeof(char));
    strcat(path, "./SDStore/");
    strcat(path, new_exec);
    return path;
}

void proc_file(int argc, char* argv[])
{
    int i, return_exec, status;
    
    char *path = NULL;

    int fd_in = open(argv[2], O_RDONLY);
    int fd_out = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0666);

    int fd_0 = dup(0);
    int fd_1 = dup(1);

    dup2(fd_in, 0);
    dup2(fd_out, 1);

    close(fd_in);
    close(fd_out);

    for(i=4; i<argc; i++)
    {
        if (fork() == 0)
        {
            path = get_path(argv[i]);
            return_exec = execl(path, argv[i], NULL);
            free(path);
            _exit(return_exec);
        }
        else
        {
            wait(&status);
        }
    }

    dup2(fd_0, 0);
    dup2(fd_1, 1);
}

int main(int argc, char* argv[])
{
    char *arg = calloc(10, sizeof(char));
    strcpy(arg, "proc-file");

    if (strcmp(argv[1], "proc-file") == 0)
    {
        proc_file(argc, argv);
    }
    return 0;
}
