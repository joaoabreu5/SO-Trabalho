#include "parser.h"
#include "readln.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define OP_SIZE 7

typedef struct operation
{
    int n_max;
    char *op;
} operation;

Operation parse(char *filename)
{
    int i = 0;
    char buffer[100], *aux, *line;
    Operation ops = malloc(OP_SIZE * sizeof(operation));
    int fd = open(filename, O_RDONLY);
    if (fd >= 0)
    {
        while (readln(fd, buffer, 100) > 0 && i < OP_SIZE)
        {
            aux = strdup(buffer);
            line = strsep(&aux, " ");
            if (line != NULL)
            {
                if (strcmp(line, "") != 0)
                {
                    ops[i].op = strdup(line);
                    if (aux != NULL)
                    {
                        ops[i].n_max = atoi(aux);
                    }
                    else
                        ops[i].n_max = -1;
                }
            }
            i++;
        }
        return ops;
    }
    write(2, "Invalid File", 13);
    return NULL;
}