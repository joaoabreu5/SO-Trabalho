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
    if ((*head)->commands.priority >= msg.priority)
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
               start->next->commands.priority <= msg.priority)
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

void print_Queue(Node *head)
{
    while (head != NULL)
    {
        printf("Priority: %d    Commands : %s", head->commands.priority, head->commands.commands);
        head = head->next;
    }
}