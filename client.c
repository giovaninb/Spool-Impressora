/***********
* Desvolvedor: Giovani Nícolas Bettoni
* Contato: giovanib@ufcspa.edu.br
***********/

#include "common.h"
#include <unistd.h>

int fd;
int errno;
int shm_exists;
Shared* shared_mem;

int setup_shared_memory(){
    //Set the file descriptor to shared memory
    fd = shm_open(t_shm, O_RDWR , S_IRWXU);
    if(fd == -1){
        printf("Could not open share memory\n");
        exit(1);
    }
    return 0;
}

int attach_shared_memory(){

    //Attach the shared memory at fd to the process virtual address
    shared_mem = (Shared*) mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shared_mem == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }
    printf("Successfully attached shared memory to printer server\n");

    return 0;
}


void catch_signal( int the_signal ) {
    int temp;
    sem_getvalue(&shared_mem->mutex, &temp);
    if(temp != 1){
        sem_post(&shared_mem->mutex);
        printf("\nExisting in critical region, set mutex back to 1\n");
    }
    printf("\nExitting safely\n");
    exit(1);
}


int main(int argc, char argv[]){
    //Set up hooks for cleaning
    if ( signal (SIGINT, catch_signal ) == SIG_ERR ){
        perror( "SIGINT failed" );
        exit (1);
    }
    if ( signal (SIGQUIT, catch_signal ) == SIG_ERR ){
        perror( "SIGQUIT failed" );
        exit(1);
    }
    setup_shared_memory();
    attach_shared_memory();

    while(1){

        //Create the job struct
        struct Job printjob;
        printf("What is the name of your job ?\n>");
        scanf("%[^\n]%*c", printjob.name);
        printf("What is the time required for your job ?\n>");
        scanf("%d", &printjob.time);
        printjob.ownerpid = getpid();
        //wait to get the mutex
        if(sem_trywait(&shared_mem->overflow)==-1){
            printf("Queue full, waiting for a job to finish\n");
            sem_wait(&shared_mem->overflow);
        }
        sem_wait(&shared_mem->mutex);
        //put the job in the queue
        shared_mem->joblist[(shared_mem->qfront)%shared_mem->queuesize] = printjob;
        printf("The job has been successfully added to the job queue.\nThere are %d jobs waiting before this job.\n",shared_mem->jobcount);

        shared_mem->qfront++;
        shared_mem->jobcount++;

        sem_post(&shared_mem->mutex);
        sem_post(&shared_mem->underflow);

        char c;
        printf("Do you want to create a new job ? [y/n]\n>");
        c = getchar();  //clear the newline char in stdin
        c = getchar();
        getchar();
        if(c == 'n' || c == 'N'){
            printf("No more job to send, exiting\n");
            exit(0);
        }
            
    }
}
