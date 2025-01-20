#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// variaveis globais que serao atribuidas pelo usuario
int ponte_capacidade; 
int total_carros;     
// // variaveis globais que realizam o sistema de condições do mutex, variando de acordo com o processamento das threads(entrada e saida dos carros na ponte)
int na_ponte = 0;        
int direcao_atual = 0;    
int esperando_norte = 0;  
int esperando_sul = 0;

// inicializando o mutex e as funcoes que sincronizam o funcionamento das threads com os valores das variaveis de condicao
pthread_mutex_t ponte_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_norte = PTHREAD_COND_INITIALIZER;    
pthread_cond_t cond_sul = PTHREAD_COND_INITIALIZER;        

// funcao que implementa a entrada da thread na regiao critica mediante os valores das variaveis de condicao, recebendo como argumento o sentido do carro
void EntrarPonte(int direcao) {

    pthread_mutex_lock(&ponte_mutex);
    // se a direcao do carro for norte
    if (direcao == 1) {
        esperando_norte++;
        // se algum carro do sentido sul estiver na ponte ou a ponte estiver cheia, o carro em questao fica em espera
        while (na_ponte == ponte_capacidade || (direcao_atual == 2 && na_ponte > 0)) {
            pthread_cond_wait(&cond_norte, &ponte_mutex);
        }
        // carro do norte entrando na ponte
        esperando_norte--;
        direcao_atual = 1; 
    // se a direcao do carro for norte
    } else { 
        esperando_sul++;
        // se algum carro do sentido norte estiver na ponte ou a ponte estiver cheia, o carro em questao fica em espera
        while (na_ponte == ponte_capacidade || (direcao_atual == 1 && na_ponte > 0)) {
            pthread_cond_wait(&cond_sul, &ponte_mutex);
        }
        // carro do sul entrando na ponte
        esperando_sul--;
        direcao_atual = 2; 
    }
    // variavel de qtd de carros na ponte eh incrementada apos o carro ter entrado
    na_ponte++; 
    pthread_mutex_unlock(&ponte_mutex);
}

// funcao que implementa a saida da thread da regiao critica, liberando as threads que foram colocadas anteriormente em espera
void SairPonte(int direcao) {
    pthread_mutex_lock(&ponte_mutex);

    na_ponte--;
    // caso nao haja mais nenhum carro na ponte e nenhum carro do sentido atual da ponte estiver esperando para entrar, libera os carros do sentido oposto da espera
    if (na_ponte == 0) {

        if (direcao == 1 && esperando_sul > 0) {
            pthread_cond_broadcast(&cond_sul);
        } else if (direcao == 2 && esperando_norte > 0) {
            pthread_cond_broadcast(&cond_norte);
        }
    }

    pthread_mutex_unlock(&ponte_mutex);
}

// funcao que chama as funcoes que representam a entrada e a saida das threads provenientes do sentido norte da regiao critica
void* CarroNorte(void* arg) {
    printf("Carro vindo do norte esta em espera\n");
    EntrarPonte(1);
    printf("Carro do norte entrou na ponte \n");
    SairPonte(1);
    printf("Carro do norte saiu da ponte.\n");
    pthread_exit(NULL);
}

// funcao que chama as funcoes que representam a entrada e a saida das threads provenientes do sentido sul da regiao critica
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
    // quantidade de carros referente a cada sentido
    int N;
    scanf("%d", &N);

    printf("Digite a capacidade maxima da ponte:\n");
    scanf("%d", &ponte_capacidade);
    // quantidade total de threads a serem processadas pelo programa
    total_carros = 2 * N;

    // inicializando os arrays de threads
    pthread_t threads_norte[N];
    pthread_t threads_sul[N];

    for (int i = 0; i < N; i++) {
        // cria as threads que representam os carros do sentido norte, estabelecendo sua rotina, atributos e argumentos passados para a funcao que representa sua rotina
        pthread_create(&threads_norte[i], NULL, CarroNorte, NULL);
        // cria as threads que representam os carros do sentido sul, estabelecendo sua rotina, atributos e argumentos passados para a funcao que representa sua rotina
        pthread_create(&threads_sul[i], NULL, CarroSul, NULL);
    }
    // Liberando os recursos utilizados pelas threads de forma sincrona apos o processamento de cada uma
    for (int i = 0; i < N; i++) {
        pthread_join(threads_norte[i], NULL);
        pthread_join(threads_sul[i], NULL);
    }
    // liberando os recursos utilizados pelo mutex e pelas variaveis de condicao
    pthread_mutex_destroy(&ponte_mutex);
    pthread_cond_destroy(&cond_norte);
    pthread_cond_destroy(&cond_sul);

    return 0;
}
