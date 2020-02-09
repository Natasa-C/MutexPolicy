#ifndef MTX_FUNCTIONS
#define MTX_FUNCTIONS

#include <pthread.h>
int mtx_open(pid_t pid);
int mtx_close(int id, pid_t pid);
int mtx_lock(int id, pid_t pid);
int mtx_unlock(int id, pid_t pid);

#endif