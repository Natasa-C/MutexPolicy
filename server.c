//  Hello World sever
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*
To compile:  	cc -o server server.c -lzmq
To run:		    ./server
*/

struct Mutex
{
    unsigned int id;
    int opened;
    int locked;
};

struct Pair
{
    long pid;
    struct Mutex mtx;
};

struct Pair assignedMtxs[10]; // contains pid-mtx_id mapping in order of mutex creation
long waitingPids[10];         // contains the processes that await the release of a mutex
int mtxsNo = 0;
int pidsNo = 0;

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

        // process buffer

        char delim[] = " ";
        char *ptr = strtok(buffer, delim);
        char funcName[50];
        strcpy(funcName, ptr);
        printf("\nfunction called: '%s'\n", funcName);

        // handle mtx_open
        if (strcmp(funcName, "mtx_open") == 0)
        {
            int position;
            // parse params mtx_open mtx_id mtx_pid
            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);

            long pid = atoi(ptr);
            printf("The number(unsigned long integer) is %ld\n", pid);

            // const unsigned int base = 10;
            // char **stringPart;
            // long pid = strtol(ptr, stringPart, base);

            // /* If the result is 0, test for an error */
            // if (pid == 0)
            // {
            //     /* If a conversion error occurred, display a message and exit */
            //     if (errno == EINVAL)
            //     {
            //         printf("Conversion error occurred: %d\n", errno);
            //         exit(0);
            //     }

            //     /* If the value provided was out of range, display a warning message */
            //     if (errno == ERANGE)
            //         printf("The value provided was out of range\n");
            // }

            // printf("String part is %s", stringPart);

            // body of mtx_open
            if (mtxsNo == 0)
            {
                // create new entry
                mtxsNo = 1;

                struct Mutex newMtx;
                newMtx.id = mtxsNo;
                newMtx.opened = 1;
                newMtx.locked = 0;

                struct Pair newPair;
                newPair.pid = pid;
                newPair.mtx = newMtx;

                assignedMtxs[mtxsNo - 1] = newPair;
                position = mtxsNo - 1;

                // send mtx_id = 1 to user
            }
            else
            {
                int mutexIsAvailable = 0;
                for (int i = 0; i < mtxsNo; i++)
                {
                    // get first mutex with opened status false
                    if (assignedMtxs[i].mtx.opened == 0)
                    {                                   // mutex was closed and can be opened again
                        assignedMtxs[i].mtx.opened = 1; // open mutex for current pid
                        assignedMtxs[i].mtx.locked = 0; // default lock value
                        assignedMtxs[i].pid = pid;      // associates current pid with the mutex
                        // send mtx_id = i + 1 to user
                        mutexIsAvailable = 1;
                        position = i;
                        break;
                    }
                }

                if (!mutexIsAvailable)
                {
                    // create new entry
                    mtxsNo += 1;

                    struct Mutex newMtx;
                    newMtx.id = mtxsNo;
                    newMtx.opened = 1;
                    newMtx.locked = 0;

                    struct Pair newPair;
                    newPair.pid = pid;
                    newPair.mtx = newMtx;

                    assignedMtxs[mtxsNo - 1] = newPair;
                    position = mtxsNo - 1;

                    // send mtx_id = mtxs_No to user
                }
            }

            printf("mtxsPosition: %d\n", position);
            printf("mtxId: %d\n", assignedMtxs[mtxsNo - 1].mtx.id);
            printf("mtxOpened: %d\n", assignedMtxs[mtxsNo - 1].mtx.opened);
            printf("mtxLocked: %d\n", assignedMtxs[mtxsNo - 1].mtx.locked);
            printf("Pid: %ld\n", assignedMtxs[mtxsNo - 1].pid);
        }
        // else if (strcmp(funcName, "mtx_close")) {  // handle mtx_close
        //     // parse params mtx_close mtx_id pid

        //     // parse mtx_id
        //     ptr = strtok(buffer, delim);
        //     printf("ptr: %s\n", *ptr);

        //     // parse pid
        //     ptr = strtok(NULL, delim);
        //     printf("ptr: %s\n", *ptr);

        //     const unsigned int base = 10;
        //     char *stringPart;
        //     long  pid = strtol(ptr, &stringPart, base);

        //     /* If the result is 0, test for an error */
        //     if (pid == 0)
        //     {
        //         /* If a conversion error occurred, display a message and exit */
        //         if (errno == EINVAL)
        //         {
        //             printf("Conversion error occurred: %d\n", errno);
        //             exit(0);
        //         }

        //         /* If the value provided was out of range, display a warning message */
        //         if (errno == ERANGE)
        //             printf("The value provided was out of range\n");
        //     }

        //     printf("The number(unsigned long integer) is %ld\n", pid);
        //     printf("String part is |%s|", stringPart);
        // }

        sleep(1);
        zmq_send(responder, "01,2", 5, 0);
    }
    return 0;
}