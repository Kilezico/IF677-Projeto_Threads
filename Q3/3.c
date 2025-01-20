#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int ponte_capacidade; 
int total_carros;     

int na_ponte = 0;        
int direcao_atual = 0;    
int esperando_norte = 0;  
int esperando_sul = 0;

pthread_mutex_t ponte_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_norte = PTHREAD_COND_INITIALIZER;    
pthread_cond_t cond_sul = PTHREAD_COND_INITIALIZER;          

void EntrarPonte(int direcao) {
    pthread_mutex_lock(&ponte_mutex);

    if (direcao == 1) {
        esperando_norte++;
        while (na_ponte == ponte_capacidade || (direcao_atual == 2 && na_ponte > 0)) {
            pthread_cond_wait(&cond_norte, &ponte_mutex);
        }
        esperando_norte--;
        direcao_atual = 1; 
    } else { 
        esperando_sul++;
        while (na_ponte == ponte_capacidade || (direcao_atual == 1 && na_ponte > 0)) {
            pthread_cond_wait(&cond_sul, &ponte_mutex);
        }
        esperando_sul--;
        direcao_atual = 2; 
    }

    na_ponte++; 
    pthread_mutex_unlock(&ponte_mutex);
}


void SairPonte(int direcao) {
    pthread_mutex_lock(&ponte_mutex);

    na_ponte--; 
    if (na_ponte == 0) {
       
        if (direcao == 1 && esperando_sul > 0) {
            pthread_cond_broadcast(&cond_sul);
        } else if (direcao == 2 && esperando_norte > 0) {
            pthread_cond_broadcast(&cond_norte);
        }
    }

    pthread_mutex_unlock(&ponte_mutex);
}

void* CarroNorte(void* arg) {
    printf("Carro vindo do norte esta em espera\n");
    EntrarPonte(1);
    printf("Carro do norte entrou na ponte \n");
    SairPonte(1);
    printf("Carro do norte saiu da ponte.\n");
    pthread_exit(NULL);
}


void* CarroSul(void* arg) {
    printf("Carro vindo do sul esta em espera\n");
    EntrarPonte(2);
    printf("Carro do sul entrou na ponte.\n");
    SairPonte(2);
    printf("Carro do sul saiu da ponte.\n");
    pthread_exit(NULL);
}

int main() {
    printf("Digite o número de carros para cada sentido:\n");
    int N;
    scanf("%d", &N);

    printf("Digite a capacidade maxima da ponte:\n");
    scanf("%d", &ponte_capacidade);

    total_carros = 2 * N;

    pthread_t threads_norte[N];
    pthread_t threads_sul[N];

    for (int i = 0; i < N; i++) {
        pthread_create(&threads_norte[i], NULL, CarroNorte, NULL);

        pthread_create(&threads_sul[i], NULL, CarroSul, NULL);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads_norte[i], NULL);
        pthread_join(threads_sul[i], NULL);
    }

    pthread_mutex_destroy(&ponte_mutex);
    pthread_cond_destroy(&cond_norte);
    pthread_cond_destroy(&cond_sul);

    return 0;
}
