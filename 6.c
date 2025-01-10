#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_TAMANHO 10

int array[ARRAY_TAMANHO];
int contador = 0; //Contador de threads (Se for -1, há um escritor no array, se for > 0, há threads leitoras no array)
int posicaoLer = 0; //Deve ser escolhida aleatoriamente
int posicaoEscrever = 0; //Deve ser escolhida aleatoriamente

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ler = PTHREAD_COND_INITIALIZER;
pthread_cond_t escrever = PTHREAD_COND_INITIALIZER;

void *leitor(void *id) {
    int identificacao = *(int *)id;
    while(1) {
        pthread_mutex_lock(&mutex);

        while(contador < 0) { //Enquanto há algum escritor atuando no array
            pthread_cond_wait(&ler, &mutex);
        }
        contador ++;

        pthread_mutex_unlock(&mutex);

        //Escolhendo alguma posição aleatória do array para ler
        posicaoLer = rand() % ARRAY_TAMANHO;
        int valorLido = array[posicaoLer];
        printf("Olá! Sou a thread leitora %d e li o valor %d na posição %d\n", identificacao, valorLido, posicaoLer);

        pthread_mutex_lock(&mutex);

        contador --;
        if(contador == 0) {
            pthread_cond_signal(&escrever);
        }

        pthread_mutex_unlock(&mutex);

        for(int i = 0; i < 1000000; i ++);  //Atrasa um pouco as funções, dando chance para outros tipos de threads serem executados
    }

    pthread_exit(NULL);
}

void *escritor(void *id) {
    int identificacao = *(int *) id;
    while(1) {
        pthread_mutex_lock(&mutex);

        //Espera o sinal que indica que não há mais threads leitoras ou escritoras no array
        while(contador != 0) {
            pthread_cond_wait(&escrever, &mutex);
        }
        contador = -1;

        pthread_mutex_unlock(&mutex);
        
        //Escolhendo alguma posição aleatória do array para escrever
        posicaoEscrever = rand() % ARRAY_TAMANHO;
        array[posicaoEscrever] = rand() % 10; //Escreve algum valor aleatório entre 0 e 9
        printf("Olá! Sou a thread escritora %d e estou escrevendo na posição %d\n", identificacao, posicaoEscrever);

        pthread_mutex_lock(&mutex);

        contador = 0; //Thread escritora saiu da região crítica, portanto leitores podem entrar, bem como algum outro escritor
        pthread_cond_broadcast(&ler);
        pthread_cond_signal(&escrever); //Apenas acordar uma escritora
        
        pthread_mutex_unlock(&mutex);

        for(int i = 0; i < 1000000; i ++); //Atrasa um pouco as funções, dando chance para outros tipos de threads serem executados
    }

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));
    int N, M; //Números de threads leitoras e escritoras, respectivamente, a serem informados pelo usuário
    scanf("%d %d", &N, &M);
    if(N <= 0 || M <= 0) {
        printf("Erro! Números de threads leitoras e escritoras devem ser positivos.\n");
        return 1;
    }

    //Alocação dinâmica das threads
    pthread_t *leitoras = (pthread_t *) malloc(N * sizeof(pthread_t));
    pthread_t *escritoras = (pthread_t *) malloc(M * sizeof(pthread_t));
    int *ids = (int*) malloc((N + M) * sizeof(int));
    if(!leitoras || !escritoras || !ids) {
        printf("Erro na alocação de memória!\n");
        return 1;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&ler, NULL);
    pthread_cond_init(&escrever, NULL);

    for(int i = 0; i < N; i ++) { //Criando threads leitoras
        ids[i] = i + 1;
        pthread_create(&leitoras[i], NULL, leitor, &ids[i]);
    }

    for(int i = 0; i < M; i ++) { //Criando threads escritoras
        ids[N + i] = i + 1; //Serão criadas após as leitoras, portanto, seus IDs são somados a N
        pthread_create(&escritoras[i], NULL, escritor, &ids[N + i]);
    }

    //A main precisa esperar as threads (Contudo, nunca vai acontecer, uma vez que as threads não têm fim)
    for(int i = 0; i < N; i ++) {
        pthread_join(leitoras[i], NULL);
    }

    for(int i = 0; i < M; i ++) {
        pthread_join(escritoras[i], NULL);
    }

    //Liberando memória
    free(leitoras);
    free(escritoras);
    free(ids);

    pthread_exit(NULL);
}