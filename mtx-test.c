#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zmq.h>

pid_t getpid(void);
pid_t getppid(void);

int mtx_open(int id, pid_t pid){

    printf("a");
    // connect to server
    void *context = zmq_ctx_new();
    if(context == NULL)
        return -1;
    
    void *requester = zmq_socket(context, ZMQ_REQ);
    if(requester == NULL)
        return -1;

    if(zmq_connect(requester, "tcp://localhost:5555") != 0){
        return -1;
    };

    // create buffer to be sent
    char mypid[15];
    sprintf(mypid, "%d", pid);
    char myid[15];
    sprintf(myid, "%d", id);
    char buffer[50];
    strcpy(buffer, "mtx_open");
    strcat(buffer, " ");
    strcat(buffer, myid); // mutex id
    strcat(buffer, " ");
    strcat(buffer, mypid); // process pid

    // send to daemon

    zmq_send(requester, buffer, 50, ZMQ_DONTWAIT);
    printf("a");

    // receive from server
    char received_text[2];   
    zmq_recv(requester, received_text, 2, 0);
    printf("a");
    int status = 0;     // 0 on success, -1 on failure
    if(strstr(received_text, "-")){
        status = -1;
    } 

    zmq_close(requester);
    zmq_ctx_destroy(context);

    return status; 
}

int main(){

    int id = 5;
    pid_t pid = getpid();
    printf("a");
    int c = mtx_open(id, pid);
    return 0;
}