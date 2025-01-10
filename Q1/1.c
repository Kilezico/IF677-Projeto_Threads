#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char palavra[100]; //variavel global contendo a palavra
int d = 0, e = 0;//variaveis globais onde d contem o numero de arquivos terminados, e controla a quantidade de threads ativas

void* Procurar(void* search){
FILE* file; //criando variaveis
file = fopen((char*) search, "r"); //leitura do arquivo
int f = 0, i = 0; //f representa a linha atual do arquivo e i representa o indice da palavra
char frase[200];//variavel da frase
int cond = 0; //condicao para checar se as letras sao iguais
while(fgets(frase, 200, file) != NULL){ //loop para coletar as linhas do arquivo
f++; //a linha atual e atualizada
for(int c = 0; c < strlen(frase); c++){ //loop para chegar ate o final da linha passando letra por letra
if(!cond){i = 0;} else{i++;} //se a condicao for 1, o indice i e incrementado, e caso contrario, o indice i e zerado
if(i == strlen(palavra)){printf("<%s>:<%d>\n",(char*) search,f); i = 0; cond = 0; continue;} //se a palavra for encontrada, sera printado o nome do arquivo e a linha, e o loop sera continuado a partir da proxima iteracao
if(frase[c] == palavra[i]){cond = 1;} else{cond = 0;} //se as letras forem iguais, a condicao e 1
}
}
d++; //numeros de arquivos finalizados aumenta
e--; // numero de threads disponiveis aumenta
pthread_exit(NULL);
}

int main()
{   int file_number = 0, thread_number = 0, b = 0, c = 0;
    //coletando variaveis como numero de arquivos, threads e a palavra desejada
    printf("Defina o numero de arquivos: "); scanf("%d", &file_number);
    printf("Defina o numero de threads: "); scanf("%d", &thread_number);
    printf("Palavra: ");  scanf("%s", palavra);
    //criando matriz de char para coletar os nomes dos arquivos
    char **nomes = (char**) calloc(file_number, sizeof(char*));
    for(int a = 0; a < file_number; a++){nomes[a] = (char*) calloc(100, sizeof(char));}
    for(int a = 0; a < file_number; a++){
        printf("Diga o nome do %do arquivo: ", a+1); scanf("%s", nomes[a]);
    }
    //threads
    pthread_t *threads = (pthread_t*) calloc(thread_number, sizeof(pthread_t));
   //caso haja mais threads disponiveis do que arquivos
   if(thread_number > file_number){thread_number = file_number;}
   while(d < file_number){//loop que terminara somente quando todos arquivos forem lidos
    for(; (e < thread_number) && (c < file_number); e++){ //for que sempre ativara quando houver threads disponiveis
       b = pthread_create(&threads[e], NULL, Procurar, (void*) nomes[c]); c++; //criando threads e atualizando a variavel c para ser usado o proximo arquivo
        if(b){ //checando se houve erro nas threads
            printf("Erro e o retorno e %d\n", b);
            exit(-1);
        }
    }
   }
   pthread_exit(NULL);
}
