#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zmq.h>

/*
To compile:  	cc -o mtx mtx-test.c -lzmq
To run:		    ./mtx
*/

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
        return status;
    }
    
    printf("Status: %d\n", status);
    int mtx_id = atoi(received_text);
    printf("Pid %s opened mutex %d\n", mypid, mtx_id);

    //close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return mtx_id;
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
        printf("Status: %d\n", status);
        return -1;
    }

    //close socket
    zmq_close(requester);

    //destroy context
    zmq_ctx_destroy(context);

    printf("Status: %d\n", status);
    printf("Pid %s closed mutex %s\n", mypid, myid);
    return 0;
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

    int status = 0;

    while (1)
    { // waits until it receives the mutex

        char received_text[2];
        zmq_recv(requester, received_text, 2, 0);

        if (strstr(received_text, "-"))
        {
            status = -1; // if a error has occurred -1 is returned
            break;
        }
        else if (strstr(received_text, "0"))
        {
            // succesfull function => received mutex
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
        zmq_send(requester, partialBuffer, 50, 0);
    }

    //close socket
    zmq_close(requester);
    //destroy context
    zmq_ctx_destroy(context);

    printf("Status: %d\n", status);
    printf("Pid %s locked mutex %s\n", mypid, myid);
    // if an error occurred
    if (status == -1)
        return -1;

    return 0;
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
    }

    //close socket
    zmq_close(requester);

    //destroy context
    zmq_ctx_destroy(context);

    printf("Status: %d\n", status);
    if (status == 0)
        printf("Pid %s unlocked mutex %s\n", mypid, myid);
    return status;
}

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
        // mtx_close(1, 9146);
        mtx_lock(id, pid);
        mtx_unlock(id, pid);
        mtx_close(id, pid);
    }

    return 0;
}
