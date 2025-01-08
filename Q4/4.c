#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define P 1    // Qtd. de threads produtoras
#define C 1    // Qtd. de threads consumidoras
#define B 10    // Tamanho do Buffer da BlockingQueue

typedef struct elem{
    int value;
    struct elem *prox;
} Elem;
 
typedef struct blockingQueue{
   unsigned int sizeBuffer, statusBuffer;
   Elem *head,*last;
} BlockingQueue;

typedef struct thread_args {
    int id;
    BlockingQueue *queue;
} ThreadArgs;

BlockingQueue *newBlockingQueue(unsigned int SizeBuffer);
void putBlockingQueue(BlockingQueue *Q, int newValue, int id);
int takeBlockingQueue(BlockingQueue *Q, int id);

pthread_mutex_t mutexLegal = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cheia = PTHREAD_COND_INITIALIZER;
pthread_cond_t vazia = PTHREAD_COND_INITIALIZER;

void *produtor(void *args);
void *consumidor(void *args);

int main()
{
    // Cria a fila
    BlockingQueue *bQueue = newBlockingQueue(B);
    // Cria as threads
    pthread_t produtores[P];
    pthread_t consumidores[C];
    for (int i=1; i<=P; i++) {
        // Aloca memória para os argumentos (struct porque temos mais de um argumento)
        ThreadArgs *args = (ThreadArgs *) malloc(sizeof(ThreadArgs));
        args->id = i; args->queue = bQueue;
        pthread_create(&produtores[i], NULL, produtor, (void *) args);
    }
    for (int i=1; i<=C; i++) {
        ThreadArgs *args = (ThreadArgs *) malloc(sizeof(ThreadArgs));
        args->id = i; args->queue = bQueue;
        pthread_create(&consumidores[i], NULL, consumidor, (void *) args);
    }

    pthread_exit(NULL);
}

void *produtor(void *args)
{
    int id = ((ThreadArgs *) args)->id;
    BlockingQueue *queue = ((ThreadArgs *) args)->queue;
    printf("Produtor %d: Nasci! Pronto para o trabalho.\n", id);
    
    int produto = 1;
    while (1) {
        // Produz e tenta colocar na fila
        putBlockingQueue(queue, produto, id);
        printf("Produtor %d: produzi %d.\n", id, produto);
        produto++;
    }

    printf("Produtor %d: Saindo! Ja chegou minha hora.\n", id);
    pthread_exit(NULL);
}

void *consumidor(void *args)
{
    int id = ((ThreadArgs *) args)->id;
    BlockingQueue *queue = ((ThreadArgs *) args)->queue;
    printf("Consumidor %d: Nasci! Pronto para o trabalho.\n", id);
    
    while (1) {
        // Tenta e pegar da fila pra consumir
        int prod = takeBlockingQueue(queue, id);
        printf("Consumidor %d: consumi %d.\n", id, prod);
    }

    printf("Consumidor %d: Saindo! Ja chegou minha hora.\n", id);
    pthread_exit(NULL);
}

BlockingQueue *newBlockingQueue(unsigned int SizeBuffer)
{
    // Cria nova fila e retorna
    BlockingQueue *bq = (BlockingQueue *) malloc(sizeof(BlockingQueue));
    bq->head = NULL;
    bq->last = NULL;
    bq->sizeBuffer = SizeBuffer;
    bq->statusBuffer = 0;
    return bq;
}

void putBlockingQueue(BlockingQueue *Q, int newValue, int id)
{
    pthread_mutex_lock(&mutexLegal);
    // Para a thread se a fila estiver cheia
    while (Q->statusBuffer == Q->sizeBuffer) {
        printf("Produtor %d: Fui dormir!\n", id);
        pthread_cond_wait(&cheia, &mutexLegal);
    }

    // Quando a fila não estiver mais cheia, coloca o novo item
    Elem *novo_elem = (Elem *) malloc(sizeof(Elem));
    novo_elem->value = newValue;
    novo_elem->prox = NULL;
    if (Q->last != NULL) 
        // Se tem alguem na fila, fim antigo aponta pra novo_elem (novo fim)
        Q->last->prox = novo_elem;
    else 
        // Se está vazia, novo_elem é novo começo também.
        Q->head = novo_elem;
    Q->last = novo_elem;
    Q->statusBuffer++;

    if (Q->statusBuffer == 1) {
        // Estava vazia, acorda consumidores dormindo
        pthread_cond_broadcast(&vazia);
    }
    pthread_mutex_unlock(&mutexLegal);
}

int takeBlockingQueue(BlockingQueue *Q, int id)
{
    pthread_mutex_lock(&mutexLegal);
    // Para a thread se a flia esiver vazia
    while (Q->statusBuffer == 0) {
        printf("Consumidor %d: Fui dormir!\n", id);
        pthread_cond_wait(&vazia, &mutexLegal);
    }

    // Quando a fila não estiver vazia, pega o primeiro elemento
    Elem *front = Q->head;
    Q->head = Q->head->prox;
    if (Q->head == NULL) Q->last = NULL; // Se for o ultimo elemento da fila, também seta o Q->last;
    int value = front->value;
    free(front);

    Q->statusBuffer--;
    if (Q->statusBuffer == Q->sizeBuffer - 1) {
        // Estava cheia, acorda produtores dormindo
        pthread_cond_broadcast(&cheia);
    }
    pthread_mutex_unlock(&mutexLegal);

    return value;
}