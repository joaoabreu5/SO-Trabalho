#include "parser.h"
#include "readln.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define OP_SIZE 7

/*
    Function that takes as a parameter a string that represents the name of the file we want to read.
    If the file exists, every line is parsed and added to an array of operation and the array is returned,
    Else an error message is written in the stderr and NULL is returned
*/
Operation parse(char *filename)
{
    int i = 0;
    char buffer[100], *aux, *line;
    Operation ops = NULL;
    int fd = open(filename, O_RDONLY);
    if (fd >= 0)
    {
        ops = malloc(OP_SIZE * sizeof(operation));
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
                free(line);
            }
            i++;
        }
    }
    else
        write(2, "Error: The file does not exist.\n", 33);
    close(fd);
    return ops;
}

/*
    If opv exists it frees every string alocates and returns 0, else returns -1
*/
int free_Operation(Operation opv)
{
    if (opv != NULL)
    {
        for (int i = 0; i < OP_SIZE; i++)
        {
            free(opv[i].op);
        }
        return 0;
    }
    return -1;
}

/*
    If op1 exists in the operation array, returns the maximum number of simultaneous operations
    that op1 can do, else returns -1;
*/
int get_nr_max(char *op1, Operation opv)
{
    int found = 0, ret = -1, i = 0;
    while (i < OP_SIZE && !found)
    {
        if (strcmp(opv[i].op, op1) == 0)
        {
            ret = opv[i].n_max;
            found = 1;
        }
        i++;
    }
    return ret;
}