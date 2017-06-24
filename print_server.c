/***********
* Desvolvedor: Giovani Nícolas Bettoni
* Contato: giovanib@ufcspa.edu.br
***********/

#include "common.h"
#include <unistd.h>

#define PBSTR "############################################################"
#define PBWIDTH 60

int fd;
int errno;
int shm_exists;
Shared* shared_mem;

int setup_shared_memory(){
    //Set the file descriptor fd to the shared mem
    //the memory will have the size only of the specified object
    //->Only create the shared memory if it does not exists
    fd = shm_open(t_shm, O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
    if(errno == EEXIST){
        printf("Shared memory already exists, opening instead of creating\n");
        shm_exists = 1;
        fd = shm_open(t_shm, O_RDWR, S_IRWXU);
    }
    if(fd == -1){
        //Check failed memory assignment
        printf("Could not open or create share memory\n");
        exit(1);
    }
    ftruncate(fd, sizeof(Shared));

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

int init_shared_memory(){
    
    int temp;

    printf( "Enter the size of the job queue,\nmust be an integer between 0 and 50:\n>");
    scanf("%d" , &temp);
    while( temp < 1 || temp > 50){
        printf( "Invalid input, please enter an integer between 0 and 50\n>" );
        scanf("%d" , &temp);
    }


    //Here init everything in the struct object
    shared_mem->qfront = 0;
    shared_mem->qrear = 0;
    shared_mem->jobcount = 0;
    shared_mem->queuesize = temp;
    sem_init(&(shared_mem->underflow), 1, 0);// Underflow start at 0
    sem_init(&(shared_mem->overflow), 1, temp); // Overflow starts at max
    sem_init(&(shared_mem->mutex), 1, 1);  // first 1 means shared between processes

    printf("Printer server initialized shared memory with a capacity of %d jobs\n",shared_mem->queuesize);

}

void catch_signal( int the_signal ) {
    signal( the_signal, catch_signal );
    printf( "\nSignal %d received\n", the_signal );
    if(the_signal == SIGQUIT || the_signal == SIGINT ){
        printf( "Cleaning and exiting\n");
        //Clean: Destroy the semaphores and unlink the shared mem
        int r=0;
        r+=sem_destroy(&(shared_mem->underflow));
        r+=sem_destroy(&(shared_mem->overflow));
        r+=sem_destroy(&(shared_mem->mutex));
        if(r){
            printf("Could not destroy semaphores\n");
        }else{
            printf("Successfully destroyed semaphores\n");
        }
        if(shm_unlink(t_shm)==-1){
            printf("Could not unlink the shared memory\n");
            exit(1);
        }
        printf("Successfully unlinked shared memory\n");
        exit(3);
    }
}

void printProgress (double percentage)
{
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf ("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush (stdout);
    printf("\n");
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

    shm_exists = 0;     //var that notes if the shm has to be init

    setup_shared_memory();      //Create or open the shared memory
    attach_shared_memory();
    if(!shm_exists){            //init shm only if first server
        init_shared_memory();   //also init the semaphores
    }else{
        printf("Skipped initialization since using already created shared memory\n");
    }

    while(1){
        int jobnum;
        //wait to get the mutex
        sem_wait(&shared_mem->underflow);
        sem_wait(&shared_mem->mutex);

        //execute a print job and print info
        jobnum = shared_mem->qrear;         //update job number
        struct Job currentjob = shared_mem->joblist[shared_mem->qrear%shared_mem->queuesize];       //load the job in a local struct
        printf("\n----------\nProcessing job #%d\nJob name: \"%s\"\nJob owner: %d\nJob time: %d\n",shared_mem->qrear, currentjob.name, currentjob.ownerpid, currentjob.time);

        shared_mem->qrear++;            //update the rear of the job queue (in an array)
        shared_mem->jobcount--;         //There is one less job in the queue

        //Exit the critical region before processing the job (no need to stay in)
        sem_post(&shared_mem->mutex);
        sem_post(&shared_mem->overflow);

        // Not working PB
        // printProgress(currentjob.time/10);      //Show PB of the task
        sleep(currentjob.time);     //Execute the job       
        printf("Job #%i done. \n----------\n\n", jobnum);
    }
}
