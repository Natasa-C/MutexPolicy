# MutexPolicy

Userland daemon to decide the policy access to the mutexes created with the help of a new set of functions: mtxopen, mtxclose, mtxlock, mtxunlock. Mutexes are visible to any process in the system.
Inter-process communication (IPC) between server and processes is ensured by the use of ZeroMQ, a high-performance asynchronous messaging library, aimed at use in distributed or concurrent applications. 

## Compile and run
### server.c
> To compile:  	cc -o server server.c -lzmq

> To run:		    ./server

### mtx_test.c
> To compile:  	cc mtx-test.c mtx_functions.c -o mtx -lzmq

> To run:		    ./mtx

### mtx_test2.c
> To compile:  	cc mtx-test2.c mtx_functions.c -o mtx -lzmq

> To run:		    ./mtx2

## The server can successully manage one or more scripts running at the same time (which better illustrates the policy of access to mutex)

## Test output for the scripts: mtx-test.c and mtx-test2.c, running at the same time
![Test-output](https://user-images.githubusercontent.com/57111995/74101258-a404f580-4b40-11ea-9e14-79868f9a5cfc.png)

## Resurces for daemon creation
- http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
- https://github.com/pasce/daemon-skeleton-linux-c
- https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux
- http://www.enderunix.org/docs/eng/daemon.php
- https://nullraum.net/how-to-create-a-daemon-in-c/

## Resurces for ZMQ
- API Reference: http://api.zeromq.org/2-1:_start
- example: https://zeromq.org/get-started/?language=c&fbclid=IwAR3A5rm8QBD2u1hcLcif5eDhA-_FdOgg8TXBGeYXFvvtFoov_4THd770VjI
