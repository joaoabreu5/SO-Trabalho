#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>  /* O_RDONLY, O_WRONLY, O_CREAT, O_* */
#include <errno.h>
#include "readln.h"

#define MAX_READ_BUFFER 10

char read_buffer[MAX_READ_BUFFER];
int read_buffer_pos = 0, read_buffer_end = MAX_READ_BUFFER;

int readc(int fd, char *c)
{
    if (read_buffer_pos == read_buffer_end)
    {
        read_buffer_end = read(fd, read_buffer, MAX_READ_BUFFER);
        switch (read_buffer_end)
        {
        case -1:
            return -1;
            break;
        case 0:
            return 0;
        default:
            read_buffer_pos = 0;
            break;
        }
    }
    *c = read_buffer[read_buffer_pos++];

    return 1;
}

ssize_t readln(int fd, char *line, size_t size)
{
    int i = 0;
    char ch;
    while (i < size && readc(STDIN_FILENO, &ch) > 0)
    {
        line[i] = ch;
        i++;
        if (ch == '\n')
            break;
    }

    return (size_t)i;
}
