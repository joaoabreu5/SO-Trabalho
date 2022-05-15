#include "declarations.h"

typedef struct node
{
    message commands;
    struct node *next;
} Node;

typedef struct list
{
    message commands;
    struct list *next;
} List;

Node *newNode(message msg);
message peek(Node **head);
void pop(Node **head);
void push(Node **head, message msg);
int isEmpty(Node **head);
void print_Queue(Node **head);
int contains(List *temp, pid_t ppid);
void remove_elem(List **temp, pid_t ppid);
void add_elem(List **temp, message msg);