#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define thread_number 1
#define file_number 1

typedef struct{
    char nomeados[100]; //variavel que ira adquirir o nome do arquivo
    int indice; //variavel que ira conter o indice das threads disponiveis    
} CharInt; //struct contendo um char e um int

char palavra[100]; //variavel global contendo a palavra
int d = 0, e = 0;//variaveis globais onde d contem o numero de arquivos terminados, e controla a quantidade de threads ativas
pthread_t threads[thread_number]; //array contendo o numero de threads
int threads_ocupadas[thread_number]; //array de ints para dizer quais threads estao disponiveis

void* Procurar(void* search){
int indicado = ((CharInt*) search)->indice; //passando o indice para mudar o indice certo do array de threads_ocupadas para indicar que o espaco esta disponivel
FILE* file; //criando variavel para leitura do arquivo
file = fopen(((CharInt*) search)->nomeados, "r"); //passando o nome do arquivo
int f = 0, i = 0, cond = 0; //f representa a linha atual do arquivo, i representa o indice da palavra, cond representa uma flag para analisar se as letras sao iguais 
char frase[500]; //variavel para ler as frases de cada linha do arquivo txt
    
while(fgets(frase, 500, file) != NULL){ //loop para coletar as linhas do arquivo
 f++; //a linha atual e atualizada
 for(int c = 0; c < strlen(frase); c++){ //loop para chegar ate o final da linha passando letra por letra
      if(frase[c] == palavra[i]){cond = 1;} else{cond = 0;} //se as letras forem iguais, a condicao e 1
      if(!cond){i = 0;} else{i++;} //se a condicao for 1, o indice i e incrementado, e caso contrario, o indice i e zerado
      if(i == strlen(palavra)){printf("<%s>:<%d>\n",(char*) search,f); i = 0; cond = 0;} //se a palavra for encontrada, sera printado o nome do arquivo e a linha 
  }
}
d++; e--; threads_ocupadas[indicado] = 0; //numeros de arquivos finalizados aumenta e numero de threads disponiveis aumenta, e um espaco e desocupado do array de threads
pthread_exit(NULL);
}

int main()
{   int b = 0, c = 0, liberado = -1;
    //coletando a palavra desejada
    printf("Palavra: ");  scanf("%s", palavra);
    //criando matriz de char para coletar os nomes dos arquivos
    char **nomes = (char**) calloc(file_number, sizeof(char*));
    for(int a = 0; a < file_number; a++){nomes[a] = (char*) calloc(100, sizeof(char));}
    for(int a = 0; a < file_number; a++){
        printf("Diga o nome do %do arquivo: ", a+1); scanf("%s", nomes[a]);
    }
    //threads
    for(int a = 0; a < thread_number; a++){threads_ocupadas[a] = 0;}
   while(d < file_number){//loop que terminara somente quando todos arquivos forem lidos
    
    for(int a = 0; a < thread_number; a++){ //loop para procurar um indice disponivel
        if(threads_ocupadas[a] == 0){liberado = a;}
            if(liberado > -1){break;}
    }
     if(c < file_number && liberado > -1 && e < thread_number){ //loop que sempre ativara quando houver threads disponiveis e arquivos que nao foram analisados
       CharInt *tmp; tmp = calloc(1, sizeof(CharInt)); //criando variavel do tipo CharInt
       strcpy(tmp->nomeados, nomes[c]); tmp->indice = liberado; //passando os dados para tmp
       b = pthread_create(&threads[liberado], NULL, Procurar, (void*) tmp); c++; //criando threads para ler arquivos nao lidos e atualizando a variavel c para a proxima thread usar o proximo arquivo
        if(b){ //checando se houve erro nas threads
            printf("Erro e o retorno e %d\n", b);
            exit(-1);
        }
        threads_ocupadas[liberado] = 1; liberado = -1; e++; //atualizando variaveis para a proxima iteracao do loop
     }
    
   }
   //liberando memoria
   for(int a = 0; a < file_number; a++){free(nomes[a]);} 
   free(nomes);
   pthread_exit(NULL);
}
