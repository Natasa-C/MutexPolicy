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
long waitingPids[10][10];     // contains the processes that await the release of a mutex identified with mtx_id of index
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
            // parse params mtx_open mtx_pid
            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);

            long pid = atoi(ptr);
            printf("The number(unsigned long integer) is %ld\n", pid);

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

                // send mtx_id = mtxsNo to user
                char id_to_send[10];
                sprintf(id_to_send, "%d", mtxsNo);
                zmq_send(responder, id_to_send, 10, 0);
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
                        mutexIsAvailable = 1;
                        position = i;

                        // send mtx_id = i + 1 to user
                        char id_to_send[10];
                        sprintf(id_to_send, "%d", i + 1);
                        zmq_send(responder, id_to_send, 10, 0);
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

                    // send mtx_id = mtxsNo to user
                    char id_to_send[10];
                    sprintf(id_to_send, "%d", mtxsNo);
                    zmq_send(responder, id_to_send, 10, 0);
                }
            }

            printf("mtxsPosition: %d\n", position);
            printf("mtxId: %d\n", assignedMtxs[mtxsNo - 1].mtx.id);
            printf("mtxOpened: %d\n", assignedMtxs[mtxsNo - 1].mtx.opened);
            printf("mtxLocked: %d\n", assignedMtxs[mtxsNo - 1].mtx.locked);
            printf("Pid: %ld\n", assignedMtxs[mtxsNo - 1].pid);
        }
        else if (strcmp(funcName, "mtx_close") == 0)
        { // handle mtx_close
            // parse params mtx_close mtx_id pid
            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            int mtx_id = atoi(ptr);
            printf("Parsed mtx_id: %d\n", mtx_id);

            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            long pid = atoi(ptr);
            printf("Parsed pid (unsigned long integer): %ld\n", pid);

            // verify mutex is opened and uncloked before close
            if (assignedMtxs[mtx_id - 1].mtx.locked == 1 || assignedMtxs[mtx_id - 1].mtx.opened == 0)
            {
                zmq_send(responder, "-1", 10, 0);
                continue;
            }

            assignedMtxs[mtx_id - 1].mtx.opened = 0;
            assignedMtxs[mtx_id - 1].mtx.locked = 0;

            zmq_send(responder, "0", 10, 0);
        }
        else if (strcmp(funcName, "mtx_lock") == 0)
        { // handle mtx_lock
            // parse params mtx_lock mtx_id pid
            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            int mtx_id = atoi(ptr);
            printf("Parsed mtx_id %d\n", mtx_id);

            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            long pid = atoi(ptr);
            printf("Parsed pid (unsigned long integer) is %ld\n", pid);

            // verifica conditii

            if (assignedMtxs[mtx_id - 1].mtx.locked == 1)
            {
                // mtx_id is locked => add pid in waitingPids list of mtx_id
                int length = waitingPids[mtx_id - 1][0];
                waitingPids[mtx_id - 1][length + 1] = pid;
                waitingPids[mtx_id - 1][0] = length + 1;
            }
            else
            {
                assignedMtxs[mtx_id - 1].mtx.locked = 1; // lock mutex
                assignedMtxs[mtx_id - 1].pid = pid;      // assigned pid with mutex it locked
            }

            zmq_send(responder, "0", 10, 0);
        }
        else if (strcmp(funcName, "mtx_check") == 0)
        { // handle mtx_lock
            // parse params mtx_lock mtx_id pid
            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            int mtx_id = atoi(ptr);
            printf("Parsed mtx_id %d\n", mtx_id);

            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            long pid = atoi(ptr);
            printf("Parsed pid (unsigned long integer) is %ld\n", pid);

            int pidIsWaiting = 0;
            for (int i = 1; i <= waitingPids[mtx_id - 1][0]; i++)
            {
                if (waitingPids[mtx_id - 1][i] == pid)
                {
                    // found pid in waiting list
                    pidIsWaiting = 1;
                    zmq_send(responder, "2", 2, 0);
                }
            }

            if (!pidIsWaiting)
                zmq_send(responder, "0", 2, 0);
        }
        else if (strcmp(funcName, "mtx_unlock") == 0)
        { // handle mtx_close
            // parse params mtx_unlock mtx_id pid
            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            int mtx_id = atoi(ptr);
            printf("Parsed mtx_id: %d\n", mtx_id);

            ptr = strtok(NULL, delim);
            printf("ptr: %s\n", ptr);
            long pid = atoi(ptr);
            printf("Parsed pid (unsigned long integer): %ld\n", pid);

            // verify mutex is opened and locked before close
            if (assignedMtxs[mtx_id - 1].mtx.locked == 0 || assignedMtxs[mtx_id - 1].mtx.opened == 0)
            {
                zmq_send(responder, "-1", 10, 0);
                continue;
            }

            assignedMtxs[mtx_id - 1].mtx.opened = 1;
            assignedMtxs[mtx_id - 1].mtx.locked = 0;

            // mutex is now opened and unlocked and can be assigned to another pid
            // verify waitingPids list of mtx_id - 1
            if (waitingPids[mtx_id - 1][0] == 0)
            {
                // no pid waiting for this mutex
                // do nothing
            }
            else
            {
                // assign mutex to first pid in waiting
                assignedMtxs[mtx_id - 1].pid = waitingPids[mtx_id - 1][1];
                assignedMtxs[mtx_id - 1].mtx.opened = 1;

                // remove pid from waitingList
                for (int i = 2; i <= waitingPids[mtx_id - 1][0]; i++)
                    waitingPids[mtx_id - 1][i - 1] = waitingPids[mtx_id - 1][i];

                waitingPids[mtx_id - 1][0] -= 1;
            }

            zmq_send(responder, "0", 10, 0);
        }

        sleep(1);
        zmq_send(responder, "01", 2, 0);
    }
    return 0;
}