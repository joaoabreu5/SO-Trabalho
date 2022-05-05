#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct message_cs
{
    pid_t client_pid;
    int type;
    int priority;
    char commands[1024];
} message, *Message;

#define SERVER_FIFO_NAME "server_fifo"
#define CLIENT_FIFO_NAME "client_%d_fifo"
