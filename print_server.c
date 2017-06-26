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
int existe_mem_compart;
Shared* shared_mem;

int config_memoria_compartilhada(){
    
    /*
     *Seta o descritor de arquivos(fd) para a memória compartilhada, 
     *a memória terá o tamanho apenas do objeto especificado
     -> Apenas crie a memória compartilhada se não existir
    */
    fd = shm_open(t_shm, O_CREAT | O_RDWR | O_EXCL, S_IRWXU);
    if(errno == EEXIST){
        printf("Memoria compartilhada ja existe!\nAbrindo ao inves de criar...\n");
        existe_mem_compart = 1;
        fd = shm_open(t_shm, O_RDWR, S_IRWXU);
    }
    if(fd == -1){
        // Verificar a tarefa de memória com falha
        printf("Não pode abrir ou criar uma memori compartilhada\n");
        exit(1);
    }
    ftruncate(fd, sizeof(Shared));

    return 0;
}

int anexa_memoria_compart(){
    // Anexa a memoria compart no descritor de arquivo ao endereco virtual do processo
    shared_mem = (Shared*) mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(shared_mem == MAP_FAILED){
        printf("mmap() falhou\n");
        exit(1);
    }
    printf("Memoria compartilhada anexada com sucesso ao servidor da impressora\n");

    return 0;
}

int init_memoria_compart(){

    int temp;
    printf( "Entre com o tamanho da fila de trabalhos, precisa ser um inteiro entre 0 e 50:\n>>");
    scanf("%d" , &temp);
    while( temp < 1 || temp > 50){
        printf( "Entrada invalida, por favor entre com um inteiro entre 0 e 50\n>" );
        scanf("%d" , &temp);
    }


    // Aqui inicia tudo em um struct
    shared_mem->qfront = 1;
    shared_mem->qrear = 1;
    shared_mem->jobcount = 1;
    shared_mem->queuesize = temp;
    sem_init(&(shared_mem->underflow), 1, 0);// 'Underflow' inicia em 0
    sem_init(&(shared_mem->overflow), 1, temp); // 'Overflow' inicia no MAX
    sem_init(&(shared_mem->mutex), 1, 1);  //  Primeiro 1 significa memoria compart. entre processos

    printf("Servidor da impressora inicia memoria compartilhada com a capacidade de %d trbalhos\n",shared_mem->queuesize);

}

void catch_signal( int the_signal ) {
    signal( the_signal, catch_signal );
    printf( "\nSinal %d recebido\n", the_signal );
    if(the_signal == SIGQUIT || the_signal == SIGINT ){
        printf( "Limpando e saindo..\n");
        // Limpar: destroi os semaforos e desvincula da memoria compart
        int r=0;
        r+=sem_destroy(&(shared_mem->underflow));
        r+=sem_destroy(&(shared_mem->overflow));
        r+=sem_destroy(&(shared_mem->mutex));
        if(r){
            printf("Nao conseguiu destruir os semaforos.\n");
        }else{
            printf("Semaforos destruidos com sucesso!\n");
        }
        if(shm_unlink(t_shm)==-1){
            printf("Memoria compartilhada desvinculada com falha.\n");    
            exit(1);
        }
        printf("Memoria compartilhada desvinculada com sucesso!\n");
        exit(3);
    }
}

void printProgress(double percentage)
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
        perror( "SIGINT falhou" );
        exit (1);
    }
    if ( signal (SIGQUIT, catch_signal ) == SIG_ERR ){
        perror( "SIGQUIT falhou" );
        exit(1);
    }

    existe_mem_compart = 0;     //var that notes if the shm has to be init

    // cria ou abre a memoria compartilhada
    config_memoria_compartilhada();      
    anexa_memoria_compart();
    // inicia memoria compartilhada somente se primeiro server
    if(!existe_mem_compart){            
        init_memoria_compart();   //tambem inicia os semaforos
    }else{
        printf("Pulando inicializacao uma vez que a memoria compartilhada ja fora criada\n");
    }
    int jobnum;
    do{
        // espera para adquiri o mutex
        sem_wait(&shared_mem->underflow);
        sem_wait(&shared_mem->mutex);

        // executa a impressao da requisicao e a impressao das informacoes
        jobnum = shared_mem->qrear;         // update o numero da requisicao
        struct Job currentjob = shared_mem->joblist[shared_mem->qrear%shared_mem->queuesize];       // carrega a requisicao em um struct local
        printf("\n***************\nProcessando requisicao #%d\nNome da requisicao: \"%s\"\nDono da requisicao: %d\nTempo de trabalho: %d\nPrioridade: %d\n",shared_mem->qrear, currentjob.name, currentjob.ownerpid, currentjob.time, currentjob.priority);
        shared_mem->qrear++;            // update na referencia da fila de aptos (em um array)
        shared_mem->jobcount--;         // tem um trabalho a menos na fila

        // finaliza a sessao critica antes de processar um novo trabalho (nao precisa mante-la)
        sem_post(&shared_mem->mutex);
        sem_post(&shared_mem->overflow);

        // P
        // TODO corrigir ProgressBar 
        // printProgress(currentjob.time/10);      //Show PB of the task
        // Executa o trabalho
        sleep(currentjob.time);     
        printf("Requisicao #%i concluida. \n---------------------\n\n", jobnum);
    } while((jobnum+1) != shared_mem->queuesize);
}
