#include "parser.h"
#include "readln.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define OP_SIZE 7

void remove_BeginningSpaces(char *str)
{
    int i, n_spaces = 0, str_len;
    
    if (str != NULL)
    {
        str_len = strlen(str);

        for(i = 0; i < str_len && (str[i] == ' ' || str[i] == '\t'); i++)
        {
            n_spaces++;
        }

        for(i = 0; n_spaces > 0 && i < str_len-n_spaces+1; i++)
        {
            str[i] = str[i+n_spaces];
        }
    }
}

Operation parse(int fd)
{
    int i = 0;
    char *buffer, *aux, *line, *aux_free = NULL;
    Operation ops = NULL;

    buffer = malloc(sizeof(char) * 100);
    ops = calloc(1, sizeof(operation));

    while (readln(fd, buffer, 100) > 0 && i < OP_SIZE)
    {
        aux = strdup(buffer);
        remove_BeginningSpaces(aux);
        aux_free = aux;
        line = strsep(&aux, " ");
        if (line != NULL)
        {
            if (strcmp(line, "nop") == 0)
            {
                ops->nop = atoi(aux);
            }
            else if (strcmp(line, "bcompress") == 0)
            {
                ops->bcompress = atoi(aux);
            }
            else if (strcmp(line, "bdecompress") == 0)
            {
                ops->bdecompress = atoi(aux);
            }
            else if (strcmp(line, "gcompress") == 0)
            {
                ops->gcompress = atoi(aux);
            }
            else if (strcmp(line, "gdecompress") == 0)
            {
                ops->gdecompress = atoi(aux);
            }
            else if (strcmp(line, "encrypt") == 0)
            {
                ops->encrypt = atoi(aux);
            }
            else if (strcmp(line, "decrypt") == 0)
            {
                ops->decrypt = atoi(aux);
            }
        }
        if (aux_free != NULL)
        {
            free(aux_free);
        }
        i++;
    }

    free(buffer);
    return ops;
}

void print_Op(Operation op)
{
    printf("Nop -> %d\n", op->nop);
    printf("Bcompress -> %d\n", op->bcompress);
    printf("Bdecompress -> %d\n", op->bdecompress);
    printf("Gcompress -> %d\n", op->gcompress);
    printf("Gdecompress -> %d\n", op->gdecompress);
    printf("Encrypt -> %d\n", op->encrypt);
    printf("Decrypt -> %d\n", op->decrypt);
}
