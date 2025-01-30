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
    int indic;
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

typedef struct {
 int variavel1; // primeiro int
 int variavel2; // segundo int
} DuoInt; // struct para passar dois ints para a thread

pthread_t threads_ativas[N];
pthread_mutex_t mutex;
pthread_cond_t cond; // Condição para caso buffer estiver vazio
pthread_cond_t icao; // Condição para caso buffer estiver cheio 
// Buffer de execuções pendentes de funções
BuffElem buffer[BUFFER_SIZE];
int items = 0;
long long int first = 0;
int last = 0; 
int fios_ativos = 0;
int threads_ocupadas[N];
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

        return -1; // Deu bronca
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

    pthread_cond_signal(&cond); // Acordando a thread despachante

    pthread_mutex_unlock(&mutex); // Saindo do mutex
    
    return id_unico; // Retornando o ID da requisição
}

void* executora(void* args) // Executa a função do usuário e adiciona o resultado no buffer temporario
{
    int idc = ((DuoInt*) args)->variavel1;
    int idc2 = ((DuoInt*) args)->variavel2;
    int (*funexecs)(void*) = buffer[idc].funexec;
    // Executa a funcao
    int res = funexecs(args);
    // Coloca no buffer temporario
    temp[idc].value = res;
    temp[idc].finished = 1;
    threads_ocupadas[idc2] = 0;
    fios_ativos--;
    // Acorda a thread esperando pelo resultado (se estiver)
    pthread_cond_signal(&temp[idc].cond_finished);
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
        //loop para verificar se ha indices em que nao ha threads ativas
        for(int a = 0; a < N; a++){
            if(threads_ocupadas[a] == 0){liberado = a;}
            if(liberado > -1){break;}
        }
        if(fios_ativos < N && liberado > -1){
        DuoInt *temp; temp->variavel1 = first; temp->variavel2 = liberado;
        rc = pthread_create(&threads_ativas[liberado], NULL, &executora, (void*) temp);
        if(rc){printf("Erro e o codigo de saida e %d", rc); exit(-1);} //checando se houve erro na thread
        threads_ocupadas[liberado] = 1; liberado = -1;
        if(first < BUFFER_SIZE){first++;} //atualiza a variavel first para pegar a proxima funcao
        else{first = 0;}
        fios_ativos++; items--; //aumentando o numero de threads ativas e diminuindo o numero de items
        pthread_cond_signal(&icao); //acordando a agendarExecucao
        }
        pthread_mutex_unlock(&mutex); //libera o mutex mutex
        
    }  
}

int pegarResultadoExecucao(int id) // Recebe um id e bloqueia a thread até obter o resultado
{
    // Procura o ID no vetor de resultados
    int retorno;
    for (int i=0; i<TEMP_SIZE; i++) {
        if (temp[i].id == id) { // Achou o ID
            pthread_mutex_lock(&temp_mutex);
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
    return -1; // Se o id recebido existe, nunca chegará aqui.
}

int funexec(void *args) { // Função de exemplo apenas para ter algo para agendar na main
    int *valor = (int *) args; // A função receberá o endereço de memória, precisamos passar pra int para poder fazer a soma!
    return *valor + 1000; // Soma mil
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

int main()
{
    printf("Bem vindo à API da questão 5!\n\n");
    initAPI();

    int valores[5] = {0, 1, 2, 3, 4};
    int ids[5];
    int resultados[5];

    for(int i = 0; i < 5; i ++) {
        ids[i] = agendarExecucao(funexec, &valores[i]); // Agendando as funções
        if(ids[i] == -1) {
            printf("Erro ao agendar a execução!\n");
        }
        
        else {
            printf("A função de ID %d foi agendada!\n", ids[i]);
        }
    }

    for(int i = 0; i < 5; i ++) { // Buscando o resultado das execuções
        resultados[i] = pegarResultadoExecucao(ids[i]);
        printf("Resultado da soma: %d\n", resultados[i]);
    }

    return 0;
}
