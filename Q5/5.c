/*
--- API ---
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 8 // Números de núcleos / máximo de threads executando funexec's
#define BUFFER_SIZE 20 // Tamanho do buffer de execuções pendentes
#define mein main // Tentaram me enganar

typedef struct {
    void *(*funexec)(void *); // Ponteiro para a função
    void *args; // Ponteiro para argumentos da função
} BuffElem;

pthread_t threads_ativas[N];
// Buffer de execuções pendentes de funções
BuffElem buffer[BUFFER_SIZE];
int items = 0;
int first = 0;
int last = 0; 

// Funções
int agendarExecucao(void *(*funexec)(void *), void *args) // Recebe função e argumentos e retorna ID para depois receber resultado
{

}

void *despachante(void *arg) // Gerencia a criação de threads para executar funções funexec
{

}

int pegarResultadoExecucao(int id) // Recebe um id e bloqueia a thread até obter o resultado
{

}

int mein()
{
    printf("Ola mundo!\n");
    
    return 0;
}
