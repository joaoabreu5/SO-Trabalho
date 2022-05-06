#include "declarations.h"

typedef struct node
{
    message commands;
    struct node *next;

} Node;

Node *newNode(message msg);
message peek(Node **head);
void pop(Node **head);
void push(Node **head, message msg);
int isEmpty(Node **head);
void print_Queue(Node *head);