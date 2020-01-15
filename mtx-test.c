#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zmq.h>

pid_t getpid(void);
pid_t getppid(void);

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
    }

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

    // receive from server
    char received_text[2];   
    zmq_recv(requester, received_text, 2, 0);

    int status = 0;     // 0 on success, -1 on failure
    if(strstr(received_text, "-")){
        status = -1;
    }; 

    //close socket and destroy context
    zmq_close(requester);
    zmq_ctx_destroy(context);

    return status; 
}

int mtx_close(int id, pid_t pid){

    // connect to server
    void *context = zmq_ctx_new();
    if(context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if(requester == NULL)
        return -1;

    if(zmq_connect(requester, "tcp://localhost:5555") != 0){
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

    int status = 0;     // 0 on success, -1 on failure
    if(strstr(received_text, "-")){
        status = -1;
    } 

    //close socket
    zmq_close(requester);
    
    //destroy context
    zmq_ctx_destroy(context);

    return status;
}
int mtx_lock(int id, pid_t pid){

    // connect to server
    void *context = zmq_ctx_new();
    if(context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if(requester == NULL)
        return -1;

    if(zmq_connect(requester, "tcp://localhost:5555") != 0){
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
    int acquired_lock;

    while(1){ // waits until it receives the mutex

        char received_text[2];
        zmq_recv(requester, received_text, 2, 0);

        if(strstr(received_text, "-")){
            status = -1; // if a error has occurred -1 is returned
            break;
        } 
        else {
            acquired_lock = atoi(received_text); 
            if(acquired_lock == 1) //the process has the mutex
                break;
        }   
    }   
    
    //close socket
    zmq_close(requester);
    //destroy context
    zmq_ctx_destroy(context);

    // if an error occurred
    if(status == -1)
        return -1;
    
    return acquired_lock;
}

int mtx_unlock(int id, pid_t pid){

    // connect to server
    void *context = zmq_ctx_new();
    if(context == NULL)
        return -1;

    void *requester = zmq_socket(context, ZMQ_REQ);
    if(requester == NULL)
        return -1;

    if(zmq_connect(requester, "tcp://localhost:5555") != 0){
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

    int status = 0;     // 0 on success, -1 on failure

    if(strstr(received_text, "-")){
        status = -1;
    } 

    //close socket
    zmq_close(requester);
    
    //destroy context
    zmq_ctx_destroy(context);
    
    return status;
}

int main(){

    int id = 5;
    pid_t pid = getpid();
    int c = mtx_open(id, pid);
    return 0;
}
