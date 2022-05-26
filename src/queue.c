#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

Node *newNode(message msg)
{
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->commands = msg;
    temp->next = NULL;
    return temp;
}

message peek(Node **head)
{
    return (*head)->commands;
}

void pop(Node **head)
{
    Node *temp = *head;
    (*head) = (*head)->next;
    free(temp);
}

void push(Node **head, message msg)
{
    Node *start = (*head);
    Node *temp = newNode(msg);
    
    if ((*head)->commands.priority < msg.priority)
    {
        temp->next = *head;
        (*head) = temp;
    }
    else
    {
        while (start->next != NULL &&
               start->next->commands.priority >= msg.priority)
        {
            start = start->next;
        }
        temp->next = start->next;
        start->next = temp;
    }
}

int isEmpty(Node **head)
{
    return (*head) == NULL;
}

void print_Queue(Node **head)
{
    Node *temp = *head;
    while (temp != NULL)
    {
        printf("Priority: %d    Commands : %s\n", temp->commands.priority, temp->commands.commands);
        temp = temp->next;
    }
}

int contains(List *temp, pid_t ppid)
{
    int res = 0;
    while (temp != NULL && !res)
    {
        if (temp->commands.client_pid == ppid)
            res = 1;
        temp = temp->next;
    }
    return res;
}

void remove_elem(List **temp, pid_t ppid)
{
    List *aux = *temp, *aux2;
    if (aux != NULL)
    {
        if (aux->commands.client_pid == ppid)
        {
            (*temp) = (*temp)->next;
            free(aux);
        }
        else
        {
            while (aux != NULL && aux->commands.client_pid != ppid)
            {
                aux2 = aux;
                aux = aux->next;
            }
            if (aux != NULL)
            {
                aux2->next = aux->next;
                free(aux);
            }
        }
    }
}

void add_elem(List **temp, message msg)
{
    List *aux = malloc(sizeof(List));
    aux->commands = msg;
    aux->next = (*temp);
    (*temp) = aux;
}
