#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// Function to Create A New Node
Node *newNode(message msg)
{
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->commands = msg;
    temp->next = NULL;

    return temp;
}

// Return the value at head
message peek(Node **head)
{
    return (*head)->commands;
}

// Removes the element with the
// highest priority form the list
void pop(Node **head)
{
    Node *temp = *head;
    (*head) = (*head)->next;
    free(temp);
}

// Function to push according to priority
void push(Node **head, message msg)
{
    Node *start = (*head);

    // Create new Node
    Node *temp = newNode(msg);

    // Special Case: The head of list has lesser
    // priority than new node. So insert new
    // node before head node and change head node.
    if ((*head)->commands.priority < msg.priority)
    {

        // Insert New Node before head
        temp->next = *head;
        (*head) = temp;
    }
    else
    {

        // Traverse the list and find a
        // position to insert new node
        while (start->next != NULL &&
               start->next->commands.priority >= msg.priority)
        {
            start = start->next;
        }

        // Either at the ends of the list
        // or at required position
        temp->next = start->next;
        start->next = temp;
    }
}

// Function to check is list is empty
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
