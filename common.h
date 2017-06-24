#ifndef _INCLUDE_COMMON_H_
#define _INCLUDE_COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// from `man shm_open`
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <signal.h>
#include <semaphore.h>

#define t_shm "/shmname"

struct Job{
    char name[50];
    int time;
    int ownerpid;
};



typedef struct {
    //Next block is job queue info
    struct Job joblist[50];     //Max is 50, but use may restrict to less
    int qfront;
    int qrear;
    int jobcount;
    int queuesize;

    sem_t underflow;
    sem_t overflow;
    sem_t mutex;
} Shared;

#endif //_INCLUDE_COMMON_H_

