/**
 * Representation of a task in the system.
 */

#ifndef TASK_H
#define TASK_H

// representation of a task
typedef struct task
{
    char *name;
    int tid;
    int priority;
    int burst;
    int og_burst;  // used in rr original burst
    int responded; // used in rr 0 or 1
} Task;

#endif
