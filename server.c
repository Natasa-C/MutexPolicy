//  Hello World sever
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
To compile:  	cc -o server server.c -lzmq
To run:		    ./server
*/

int main(void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    int rc = zmq_bind(responder, "tcp://*:5555");
    if (rc != 0)
        return 0;

    while (1)
    {
        char buffer[50];
        zmq_recv(responder, buffer, 50, 0);
        printf("%s \n", buffer);

        // Buffer should be processed in mtx array
        
        sleep(1); 
        zmq_send(responder, "01,2", 5, 0);
    }
    return 0;
}