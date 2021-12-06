/**
 * Various list operations
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"
#include "task.h"

// add a new task to the list of tasks
void insert(struct node **head, Task *newTask)
{
    // add the new task to the list
    struct node *newnode = malloc(sizeof(struct node));

    newnode->task = newTask;
    newnode->next = *head;
    *head = newnode;
}

// delete the selected task from the list
void delete (struct node **head, Task *task)
{
    struct node *temp;
    struct node *prev;

    temp = *head;
    // special case - beginning of list
    if (strcmp(task->name, temp->task->name) == 0)
    {
        *head = (*head)->next;
    }
    else
    {
        // interior or last element in the list
        prev = *head;
        temp = temp->next;
        while (strcmp(task->name, temp->task->name) != 0)
        {
            prev = temp;
            temp = temp->next;
        }

        prev->next = temp->next;
    }
}

// traverse the list
void traverse(struct node *head)
{
    struct node *temp;
    temp = head;

    while (temp != NULL)
    {
        printf("[%s] [%d] [%d]\n", temp->task->name, temp->task->priority, temp->task->burst);
        temp = temp->next;
    }
}

// CUSTOM FUNCTIONS created by vivek 121

void append(struct node **head, Task *task)
{
    struct node *temp = (struct node *)malloc(sizeof(struct node));
    struct node *end = *head;

    temp->task = task;
    temp->next = NULL;

    if (*head == NULL)
    {
        *head = temp;
        return;
    }

    while (end->next != NULL)
        end = end->next;

    end->next = temp;
    return;
}

// ALL FUNCTIONS RELATED TO SORT
struct node *recursiveMergeSort(struct node *a, struct node *b, int type)
{
    // base cases
    if (a == NULL)
    {
        return b;
    }
    else if (b == NULL)
    {
        return a;
    }

    struct node *result = NULL;

    bool pickA = false;
    // ascending burst
    if (type == 0)
        pickA = a->task->burst <= b->task->burst;
    // descending priority
    else if (type == 1)
        pickA = a->task->priority >= b->task->priority;

    if (pickA)
    {
        result = a;
        result->next = recursiveMergeSort(a->next, b, type);
    }
    else
    {
        result = b;
        result->next = recursiveMergeSort(a, b->next, type);
    }

    return result;
}

void splitLinkedList(struct node *head, struct node **frontRef,
               struct node **backRef)
{
    // base case
    if (head == NULL || head->next == NULL)
    {
        *frontRef = head;
        *backRef = NULL;
        return;
    }

    struct node *midRef = head;
    struct node *endRef = head->next;

    // advance `endRef` two nodes, and advance `midRef` one node
    while (endRef != NULL)
    {
        endRef = endRef->next;
        if (endRef != NULL)
        {
            midRef = midRef->next;
            endRef = endRef->next;
        }
    }

    *frontRef = head;
    *backRef = midRef->next;
    midRef->next = NULL;
}

// Sort a given linked list using the merge sort algorithm based on type 0 for ascended burst and 1 for descended priority
void sort(struct node **head, int type)
{
    // base case — length 0 or 1
    if (*head == NULL || (*head)->next == NULL)
    {
        return;
    }

    struct node *begin;
    struct node *end;

    // split `head` into `begin` and `end` sublists
    splitLinkedList(*head, &begin, &end);

    // recursively sort the sublists
    sort(&begin, type);
    sort(&end, type);

    // answer = merge the two sorted lists
    *head = recursiveMergeSort(begin, end, type);
}
