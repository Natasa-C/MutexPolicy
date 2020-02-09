#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zmq.h>

#include "mtx_functions.h"

/*
To compile:  	cc mtx-test.c mtx_functions.c -o mtx -lzmq
To run:		    ./mtx
*/

int main()
{
    pid_t pid = getpid();
    int id = mtx_open(pid);
    if (id == -1)
    {
        printf("%s", "Can not create mutex\n");
    }
    else
    {
        mtx_lock(id, pid);
        sleep(20);
        mtx_unlock(id, pid);
        sleep(50);
        mtx_close(id, pid);
        sleep(20);
    }

    return 0;
}