#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

int mtx_open(int id, pid_t pid){

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
    sprintf(mypid, "%u", pid);
    char buffer[50];
    strcpy(buffer, "mtx_open");
    strcat(buffer, " ");
    strcat(buffer, mypid);

    // send to daemon
    if(zmq_send(requester, buffer, 50, 0)){
        return -1;
    }
        
    //close socket
    if(zmq_close(requester)!=0){
        return -1;
    }
    
    //destroy context
    if(zmq_ctx_destroy(context)!=0){
        return -1;
    }
    
    return 0;
    
}

