/**
 * schedule_rr.c
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
    task->og_burst = burst;
    task->responded = 0;

    append(&head, task);
}

void schedule()
{
    struct node *temp = head, *otherHead = head;
    int compTime = 0, count = 0, wait = 0, turn = 0, res = 0;

    while (otherHead != NULL)
    {
        temp = otherHead;

        while (temp != NULL)
        {
            // avg response time
            if (temp->task->responded == 0)
            {
                res = res + compTime;
                temp->task->responded = 1;
            }

            // if burst lte QUANTAM and then delete
            if (temp->task->burst <= QUANTUM)
            {
                // avg time calc
                compTime = compTime + temp->task->burst;
                turn = turn + compTime;
                wait = wait + (compTime - temp->task->og_burst);
                count = count + 1;

                // run and remove from otherHead
                run(temp->task, temp->task->burst);
                delete (&otherHead, temp->task);
            }
            // else set
            else
            {
                // compTime addition
                compTime = compTime + QUANTUM;

                run(temp->task, QUANTUM);
                temp->task->burst = (temp->task->burst - QUANTUM);
            }

            temp = temp->next;
        }
    }

    printf("\nAverage waiting time = %.2f\n", (double)wait / count);
    printf("Average turnaround time = %.2f\n", (double)turn / count);
    printf("Average response time = %.2f\n", (double)res / count);
}
