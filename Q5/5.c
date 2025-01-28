/*
--- API ---
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 8 // Números de núcleos / máximo de threads executando funexec's
#define BUFFER_SIZE 20 // Tamanho do buffer de execuções pendentes
#define TEMP_SIZE 20 // Tamanho do buffer temporario
#define mein main // Tentaram me enganar

typedef struct {
    int indic;
    int (*funexec)(void *); // Ponteiro para a função 
    void *args; // Ponteiro para argumentos da função
} BuffElem;

typedef struct {
    int id;
    int finished;
    pthread_cond_t cond_finished;
    int value;
} TempElem;

pthread_t threads_ativas[N];
pthread_mutex_t mutex;
pthread_cond_t cond; //condicao para caso buffer estiver vazio
pthread_cond_t icao; //condicao para caso buffer estiver cheio 
// Buffer de execuções pendentes de funções
BuffElem buffer[BUFFER_SIZE];
int items = 0;
int first = 0;
int last = 0; 
int fios_ativos = 0;
// Buffer temporario
TempElem temp[TEMP_SIZE];
int prox_id = 1;

// Funções
int agendarExecucao(int (*funexec)(void *), void *args) // Recebe função e argumentos e retorna ID para depois receber resultado
{

}

void* executora(void* args) // Executa a função do usuário e adiciona o resultado no buffer temporario
{
    int idc = (int) args;
    int (*funexecs)(void*) = buffer[idc].funexec;
    // Executa a funcao
    int res = funexecs(args);
    // Coloca no buffer temporario
    temp[idc].value = res;
    temp[idc].finished = 1;
    // Acorda a thread esperando pelo resultado (se estiver)
    pthread_cond_signal(&temp[idc].cond_finished);
}

void *despachante(void *arg) // Gerencia a criação de threads para executar funções funexec
{
   int rc = 0;
    while(1){
        pthread_mutex_lock(&mutex);
        while(items == 0){
        pthread_cond_wait(&cond, &mutex);    
        }
        rc = pthread_create(&threads_ativas[fios_ativos], NULL, &executora, (void*) first);
        
        
        pthread_mutex_unlock(&mutex);
    }  
}

int pegarResultadoExecucao(int id) // Recebe um id e bloqueia a thread até obter o resultado
{

}

int mein()
{
    printf("Ola mundo!\n");

    return 0;
}
