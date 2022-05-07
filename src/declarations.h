#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "parser.h"

typedef struct message_cs
{
    operation op;
    pid_t client_pid;
    int type;
    int priority;
    int n_args;
    char commands[1024];
} message, *Message;

#define SERVER_FIFO_NAME "server_fifo"
#define CLIENT_FIFO_NAME "client_%d_fifo"
