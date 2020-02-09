#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zmq.h>

#include "mtx_functions.h"

pid_t getpid(void);
pid_t getppid(void);

int mtx_open(pid_t pid)
{

    // connect to server
    void *context = zmq_ctx_new();
    if (context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if (requester == NULL)
        return -1;

    if (zmq_connect(requester, "tcp://localhost:5555") != 0)
    {
        return -1;
    }

    // create buffer to be sent
    char mypid[15];
    sprintf(mypid, "%d", pid);
    // char myid[15];
    // sprintf(myid, "%d", id);
    char buffer[50];
    strcpy(buffer, "mtx_open");
    // strcat(buffer, " ");
    // strcat(buffer, myid); // mutex id
    strcat(buffer, " ");
    strcat(buffer, mypid); // process pid

    // send to daemon

    zmq_send(requester, buffer, 50, ZMQ_DONTWAIT);

    // receive from server
    char received_text[10];
    zmq_recv(requester, received_text, 10, 0);

    int status = 0; // 0 on success, -1 on failure
    if (strstr(received_text, "-"))
    {
        status = -1;
        printf("Status-open: %d\n", status);
        printf("Pid %s can not open any mutex\n", mypid);
    }
    else
    {
        printf("Status-open: %d\n", status);
        int mtx_id = atoi(received_text);
        printf("Pid %s opened mutex %d\n", mypid, mtx_id);
        status = mtx_id;
    }

    //close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return status;
}

int mtx_close(int id, pid_t pid)
{

    // connect to server
    void *context = zmq_ctx_new();
    if (context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if (requester == NULL)
        return -1;

    if (zmq_connect(requester, "tcp://localhost:5555") != 0)
    {
        return -1;
    }

    // create buffer to be sent
    char mypid[15];
    sprintf(mypid, "%d", pid);
    char myid[15];
    sprintf(myid, "%d", id);
    char buffer[50];
    strcpy(buffer, "mtx_close");
    strcat(buffer, " ");
    strcat(buffer, myid); // mutex id
    strcat(buffer, " ");
    strcat(buffer, mypid); // process pid

    // send to daemon
    zmq_send(requester, buffer, 50, 0);

    // receive from server
    char received_text[10];
    zmq_recv(requester, received_text, 10, 0);

    int status = 0; // 0 on success, -1 on failure
    if (strstr(received_text, "-"))
    {
        status = -1;
        printf("Status-close: %d\n", status);
        printf("Pid %s can not close mutex %s\n", mypid, myid);
    }

    //close socket
    zmq_close(requester);
    //destroy context
    zmq_ctx_destroy(context);

    if (status == 0)
    {
        printf("Status-close: %d\n", status);
        printf("Pid %s closed mutex %s\n", mypid, myid);
    }

    return status;
}

int mtx_lock(int id, pid_t pid)
{

    // connect to server
    void *context = zmq_ctx_new();
    if (context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if (requester == NULL)
        return -1;

    if (zmq_connect(requester, "tcp://localhost:5555") != 0)
    {
        return -1;
    }

    // create buffer to be sent
    char mypid[15];
    sprintf(mypid, "%d", pid);
    char myid[15];
    sprintf(myid, "%d", id);
    char buffer[50];
    strcpy(buffer, "mtx_lock");
    strcat(buffer, " ");
    strcat(buffer, myid); // mutex id
    strcat(buffer, " ");
    strcat(buffer, mypid); // process pid

    // send to daemon
    zmq_send(requester, buffer, 50, 0);

    int status = 0; // 0 on success, -1 on failure

    while (1)
    { // waits until it receives the mutex
        char received_text[2];
        zmq_recv(requester, received_text, 2, 0);

        if (strstr(received_text, "-"))
        {
            status = -1; // if a error has occurred -1 is returned
            printf("Status-lock: %d", status);
            printf("Pid %s can not lock mutex %s\n", mypid, myid);
            break;
        }
        else if (strstr(received_text, "0"))
        {
            // succesfull function => received mutex
            printf("Status-lock: %d\n", status);
            printf("Pid %s locked mutex %s\n", mypid, myid);
            break;
        }

        // create buffer to be sent
        char partialBuffer[50];
        strcpy(partialBuffer, "mtx_check");
        strcat(partialBuffer, " ");
        strcat(partialBuffer, myid); // mutex id
        strcat(partialBuffer, " ");
        strcat(partialBuffer, mypid); // process pid

        // send to daemon
        printf("Status-lock-check: %s\n", received_text);
        printf("Pid %s is waiting to lock mutex %s\n", mypid, myid);
        sleep(10);
        zmq_send(requester, partialBuffer, 50, 0);
    }

    //close socket
    zmq_close(requester);
    //destroy context
    zmq_ctx_destroy(context);

    return status;
}

int mtx_unlock(int id, pid_t pid)
{

    // connect to server
    void *context = zmq_ctx_new();
    if (context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if (requester == NULL)
        return -1;

    if (zmq_connect(requester, "tcp://localhost:5555") != 0)
    {
        return -1;
    }

    // create buffer to be sent
    char mypid[15];
    sprintf(mypid, "%d", pid);
    char myid[15];
    sprintf(myid, "%d", id);
    char buffer[50];
    strcpy(buffer, "mtx_unlock");
    strcat(buffer, " ");
    strcat(buffer, myid); // mutex id
    strcat(buffer, " ");
    strcat(buffer, mypid); // process pid

    // send to daemon
    zmq_send(requester, buffer, 50, 0);

    // receive from server
    char received_text[2];
    zmq_recv(requester, received_text, 2, 0);

    int status = 0; // 0 on success, -1 on failure

    if (strstr(received_text, "-"))
    {
        status = -1;
        printf("Status-unlock: %d\n", status);
        printf("Pid %s can not unlock mutex %s\n", mypid, myid);
    }
    else
    {
        printf("Status-unlock: %d\n", status);
        printf("Pid %s unlocked mutex %s\n", mypid, myid);
    }

    //close socket
    zmq_close(requester);
    //destroy context
    zmq_ctx_destroy(context);

    return status;
}