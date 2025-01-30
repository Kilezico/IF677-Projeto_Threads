/*
--- API ---
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 8 // Números de núcleos / máximo de threads executando funexec's
#define BUFFER_SIZE 20 // Tamanho do buffer de execuções pendentes
#define TEMP_SIZE 20 // Tamanho do buffer temporario

typedef struct {
    int indic; // Indice no buffer temporario referente a esta requisicao
    int (*funexec)(void *); // Ponteiro para a função 
    void *args; // Ponteiro para argumentos da função
} BuffElem;

typedef struct {
    int id;
    int finished; // FLAG - terminou ou não execução
    int delivered; // FLAG - recebeu ou não resultado
    pthread_cond_t cond_finished;
    int value;
} TempElem;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; // Condição para caso buffer estiver vazio
pthread_cond_t icao = PTHREAD_COND_INITIALIZER; // Condição para caso buffer estiver cheio
pthread_cond_t condicionamento = PTHREAD_COND_INITIALIZER; // condicao para indicar que ha um nucleo disponivel
pthread_t threads_ativas[N];
int threads_ocupadas[N];
// Buffer de execuções pendentes de funções
BuffElem buffer[BUFFER_SIZE];
int items = 0;
long long int first = 0;
int last = 0; 
int fios_ativos = 0;
int proximo = 0; // Usado para gerar os IDs, será sequencial para cada requisição
// Buffer temporario
TempElem temp[TEMP_SIZE];
pthread_mutex_t temp_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex que controla o acesso ao buffer temporário

// Funções
int agendarExecucao(int (*funexec)(void *), void *args) // Recebe função e seus argumentos e retorna um ID para depois receber resultado
{
    pthread_mutex_lock(&mutex); // Entrando no mutex

    // Espera caso o buffer esteja cheio até que o despachante libere espaço
    while(items == BUFFER_SIZE) {
        pthread_cond_wait(&icao, &mutex);
    }

    // Inicializando a entrada no buffer temporário
    pthread_mutex_lock(&temp_mutex);
    int ind_disponivel = -1; // Variável pra encontrar um índice disponível
    for(int i = 0; i < TEMP_SIZE; i ++) {
        if(temp[i].delivered) { // Achou!!
            ind_disponivel = i;
            
            break;
        }
    }

    if(ind_disponivel == -1) { // Não achou...
        pthread_mutex_unlock(&temp_mutex);
        pthread_mutex_unlock(&mutex);

        return -1; // Não há espaço no buffer para agendar esta requisição
    }

    int id_unico = proximo ++; // Gerando ID único

    // Adicionando a requisição na fila do buffer
    buffer[last].indic = ind_disponivel;
    buffer[last].funexec = funexec;
    buffer[last].args = args;

    // Atualizando o índice do buffer
    if(last < BUFFER_SIZE - 1) {
        last ++;
    }

    else {
        last = 0;
    }

    items++;

    // Configurando a entrada no buffer temporário
    temp[ind_disponivel].id = id_unico;
    temp[ind_disponivel].finished = 0;
    temp[ind_disponivel].delivered = 0;
    pthread_cond_init(&temp[ind_disponivel].cond_finished, NULL); // Para fazer a pegarResultadoExecucao saber quando o resultado estiver pronto (finished, toma)
    pthread_mutex_unlock(&temp_mutex);

    if (items == 1) pthread_cond_signal(&cond); // Acordando a thread despachante

    pthread_mutex_unlock(&mutex); // Saindo do mutex
    
    return id_unico; // Retornando o ID da requisição
}

void* executora(void* args) // Executa a função do usuário e adiciona o resultado no buffer temporario
{
    //passando os valores
    long long int idc = (long long int) args;
    pthread_mutex_lock(&mutex);
    int (*funexecs)(void*) = buffer[first].funexec;
    void* argumentostemporarios = buffer[first].args;
    int temp_idc = buffer[first].indic;
    if(first < BUFFER_SIZE-1){first++;} //atualiza a variavel first para pegar a proxima funcao
    else{first = 0;}
    pthread_mutex_unlock(&mutex); // Destrava o mutex para executar a funexec (que pode ser longa)
    
    // Executa a funcao
    int res = funexecs(argumentostemporarios);

    pthread_mutex_lock(&temp_mutex);
    // Coloca no buffer temporario
    temp[temp_idc].value = res;
    temp[temp_idc].finished = 1;
    // Acorda a thread esperando pelo resultado (se estiver)
    pthread_cond_signal(&temp[temp_idc].cond_finished);
    pthread_mutex_unlock(&temp_mutex);

    pthread_mutex_lock(&mutex);
    //atualiza a desocupacao do array de threads desocupadas e a quantidade de threads disponiveis aumenta
    threads_ocupadas[idc] = 0;
    fios_ativos--;
    // Acorda a despachante caso tinha o numero maximo de threads ativas
    if (fios_ativos == N - 1) pthread_cond_signal(&condicionamento);
    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void *despachante(void *arg) // Gerencia a criação de threads para executar funções funexec
{
   int rc = 0, liberado = -1;//variavel rc para receber a saída do pthread_create e liberado para coletar o indice da thread disponivel
   for(int a = 0; a < N; a++){threads_ocupadas[a] = 0;}
    while(1){
        pthread_mutex_lock(&mutex); //Entrando no mutex mutex
        //Caso nao haja funcoes no buffer, sera aguardado a criacao de novas funcoes 
        while(items == 0){
        pthread_cond_wait(&cond, &mutex);    
        }
        //caso haja o numero maximo de threads ativas, sera aguardado ate que libere um nucleo
        while(fios_ativos == N){
        pthread_cond_wait(&condicionamento, &mutex);
        }
        //loop para verificar se ha indices em que nao ha threads ativas
        for(int a = 0; a < N; a++){
            if(threads_ocupadas[a] == 0){liberado = a;}
            if(liberado > -1){break;}
        }
        if(fios_ativos < N && liberado > -1){
        long long int temp = liberado; //criando variavel temporaria para copiar o valor de liberado
        rc = pthread_create(&threads_ativas[liberado], NULL, &executora, (void*) temp);
        if(rc){printf("Erro e o codigo de saida e %d", rc); exit(-1);} //checando se houve erro na thread
        threads_ocupadas[liberado] = 1; liberado = -1; //atualizando o indice de thread_ocupadas e retornando o valor de liberado para -1
        fios_ativos++; items--; //aumentando o numero de threads ativas e diminuindo o numero de items
        if (items == BUFFER_SIZE-1) pthread_cond_signal(&icao); //acordando a agendarExecucao
        }
        pthread_mutex_unlock(&mutex); //libera o mutex mutex
    }  
    pthread_exit(NULL);
}

int pegarResultadoExecucao(int id) // Recebe um id e bloqueia a thread até obter o resultado
{
    pthread_mutex_lock(&temp_mutex);
    // Procura o ID no vetor de resultados
    int retorno;
    for (int i=0; i<TEMP_SIZE; i++) {
        if (temp[i].id == id) { // Achou o ID
            while (temp[i].finished == 0) { // Dorme até terminar de executar
                pthread_cond_wait(&temp[i].cond_finished, &temp_mutex);
            }

            retorno = temp[i].value;
            temp[i].delivered = 1; // Marca como recebido
            pthread_cond_destroy(&temp[i].cond_finished); // Libera memória da variável de condição
        
            pthread_mutex_unlock(&temp_mutex);
            return retorno;
        }
    }

    // Se id for um id que existe, nunca chegará aqui.
    pthread_mutex_unlock(&temp_mutex);
    return -1; 
}

void initAPI() // Inicialização das coisas da API
{
    // Inicia a thread despachante
    pthread_t despachante_id;
    pthread_create(&despachante_id, NULL, despachante, NULL);

    // Inicia o vetor temporário como disponível
    for (int i=0; i<TEMP_SIZE; i++) {
        temp[i].delivered = 1;
    }
}

int funexec(void *args) { // Função de exemplo apenas para ter algo para agendar na main
    int *valor = (int *) args; // A função receberá o endereço de memória, precisamos passar pra int para poder fazer a soma!
    return *valor + 1000; // Soma mil
}

int main()
{
    printf("Bem vindo à API da questão 5!\n\n");
    initAPI();

    int valores[23] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
    int ids[23];
    int resultados[23];

    for(int i = 0; i < 23; i ++) {
        ids[i] = agendarExecucao(funexec, &valores[i]); // Agendando as funções
        if(ids[i] == -1) {
            printf("Erro ao agendar a execução!\n");
        }
        
        else {
            printf("A função de ID %d foi agendada!\n", ids[i]);
        }
    }

    int erro = 0; //Variável para contar quantas requisições não foram agendadas
    for(int i = 0; i < 23; i ++) { // Buscando o resultado das execuções
        resultados[i] = pegarResultadoExecucao(ids[i]);
        if(resultados[i] >= 0) { // Houve espaço no buffer temporário para agendar a requisição
            printf("Resultado da soma: %d\n", resultados[i]);
        }
        else { // Caso o resultado seja -1, não houve espaço no buffer
            erro ++;
        }
    }
    printf("%d requisções não puderam ser agendadas!\n", erro);

    return 0;
}
