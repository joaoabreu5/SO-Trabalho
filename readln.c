#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h>  /* O_RDONLY, O_WRONLY, O_CREAT, O_* */
#include <errno.h>

#define MAX_READ_BUFFER 10

char read_buffer[MAX_READ_BUFFER];
int read_buffer_pos = 0, read_buffer_end = 0;

ssize_t readc_charbychar(int fd, char *c) // Ex.3 - Leitura de char em char
{
    int read_bytes = 0;
    read_bytes = read(fd, c, 1);
    return read_bytes;
}

ssize_t readc(int fd, char *c) // Ex.5 - Leitura de múltiplos char's
{
    if (read_buffer_pos == read_buffer_end)
    {
        read_buffer_end = read(fd, read_buffer, MAX_READ_BUFFER);
        switch (read_buffer_end)
        {
        case 0:
            return 0;
            break;

        case -1:
            return -1;
            break;

        default:
            read_buffer_pos = 0;
            break;
        }
    }
    *c = read_buffer[read_buffer_pos];
    read_buffer_pos++;
    return 1;
}

ssize_t readln(int fd, char *line, size_t size)
{
    int read_bytes = 0, aux = 0, index = 0;
    aux = readc(fd, &(line[index]));
    read_bytes += aux;
    while (line[index] != '\n' && aux != 0 && read_bytes < size)
    {
        index++;
        aux = readc(fd, &(line[index]));
        read_bytes += aux;
    }
    line[index] = '\0';
    return read_bytes;
}