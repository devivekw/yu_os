/**
 * schedule_sjf.c
 *
 * Schedule is in the format
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"

#include "util.c"
#include "cpu.h"

struct node *head;

void add(char *name, int priority, int burst)
{
    Task *task;
    task = malloc(sizeof(Task));

    task->name = name;
    task->tid = getID(name);
    task->priority = priority;
    task->burst = burst;

    append(&head, task);
}

void schedule()
{
    // sort based on burst
    sort(&head, 0);

    int count = 0, tempWait = 0, wait = 0, turn = 0, tempTurn = 0;
    struct node *temp = head;

    while (temp != NULL)
    {
        count = count + 1;

        // Wait Time
        if (temp->next != NULL)
        {
            tempWait = tempWait + temp->task->burst;
            wait = wait + tempWait;
        }

        // Turnaround time
        tempTurn = tempTurn + temp->task->burst;
        turn = turn + tempTurn;

        run(temp->task, temp->task->burst);
        temp = temp->next;
    }

    printf("\nAverage waiting time = %.2f\n", (double)wait / count);
    printf("Average turnaround time = %.2f\n", (double)turn / count);
    printf("Average response time = %.2f\n", (double)wait / count);
}