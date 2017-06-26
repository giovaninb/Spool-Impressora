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

int config_memoria_compartilhada(){
    //Set the file descriptor to shared memory
    fd = shm_open(t_shm, O_RDWR , S_IRWXU);
    if(fd == -1){
        printf("Não conseguiu abrir a memoria compartilhada\n");
        exit(1);
    }
    return 0;
}

int anexa_memoria_compart(){

    //Attach the shared memory at fd to the process virtual address
    shared_mem = (Shared*) mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shared_mem == MAP_FAILED){
        printf("mmap() failed\n");
        exit(1);
    }
    printf("Memoria compartilhada anexada com sucesso ao servidor da impressora\n");

    return 0;
}


void catch_signal( int the_signal ) {
    int temp;
    sem_getvalue(&shared_mem->mutex, &temp);
    if(temp != 1){
        sem_post(&shared_mem->mutex);
        printf("\nJa existe na regicao critica, setando mutex de volta para 1\n");
    }
    printf("\nSaindo com seguranca!\n");
    exit(1);
}


int main(int argc, char argv[]){
    //Set up hooks for cleaning
    if ( signal (SIGINT, catch_signal ) == SIG_ERR ){
        perror( "SIGINT falhou" );
        exit (1);
    }
    if ( signal (SIGQUIT, catch_signal ) == SIG_ERR ){
        perror( "SIGQUIT falhou" );
        exit(1);
    }
    config_memoria_compartilhada();
    anexa_memoria_compart();

    while(1){

        // Cria o struct de trabalho/Job
        struct Job printjob;
        printf("Como eh o nome da sua requisicao ?\n>");
        scanf("%[^\n]%*c", printjob.name);
        printf("Qual o tempo necessario para a sua requisicao(em seg) ?\n>");
        scanf("%d", &printjob.time);
        printjob.ownerpid = getpid();
        printf("Qual a prioridade da sua requisicao(1-adm|2-user) ?\n>");
        scanf("%d", &printjob.priority);
        // Espera pelo mutex
        if(sem_trywait(&shared_mem->overflow)==-1){
            printf("Fila cheia, esperando que alguma requisicao finalize.\n");
            sem_wait(&shared_mem->overflow);
        }
        sem_wait(&shared_mem->mutex);
        // Coloca o trabalho na fila
        shared_mem->joblist[(shared_mem->qfront)%shared_mem->queuesize] = printjob;
        printf("A requisicao foi adicionada com sucesso na fila. \nAinda ha %d requisicao(oes) antes desta.\n",shared_mem->jobcount);

        shared_mem->qfront++;
        shared_mem->jobcount++;

        sem_post(&shared_mem->mutex);
        sem_post(&shared_mem->underflow);

        char c;
        printf("Você deseja criar uma nova requisicao ? [y/n]\n>");
        c = getchar();  //clear the newline char in stdin
        c = getchar();
        getchar();
        if(c == 'n' || c == 'N'){
            printf("Mais nenhuma requisicao para enviar, saindo...\n");
            exit(0);
        }
            
    }
}
