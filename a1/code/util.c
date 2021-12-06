// CREATED by VIVEK WADHWANI
// Hosts UTILS needed by common for all files

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int getID(char *name) {
    int len;
    int i;
    char *temp;
    int result; 

    len = strlen(name);
    temp = strdup(name);

    for (i = 1; i < (len+1); i=i+1) {
        temp[i - 1] = name[i];
    }

    result = atoi(temp);
    free(temp);

    return result;
}
