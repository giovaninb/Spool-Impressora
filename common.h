#ifndef _INCLUDE_COMMON_H_
#define _INCLUDE_COMMON_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

// from `man shm_open`
#include <sys/mman.h>
#include <sys/stat.h>        /* Para mode constants */
#include <fcntl.h>           /* Para O_* constants */
#include <signal.h>
#include <semaphore.h>

#define t_shm "/shmname"

struct Job{
    char name[50];
    int time;
    int ownerpid;
    int priority;
};



typedef struct {
    //Proximo bloco sao informações da fila de trabalho
    struct Job joblist[50];     //Max eh 50, mas podemos restringir para menos
    int qfront;
    int qrear;
    int jobcount;
    int queuesize;

    sem_t underflow;
    sem_t overflow;
    sem_t mutex;
} Shared;

#endif //_INCLUDE_COMMON_H_

