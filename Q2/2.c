#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


// Matriz
char matriz[3][4];

void *checa_linhas(void *cntdr)
{
    printf("Analisando as linhas\n");
    int *contador = (int *) cntdr;
    for (int i=0; i<3; i++) {
        if (matriz[i][0] == matriz[i][1] && matriz[i][1] == matriz[i][2]) {
            if (matriz[i][0] == 'X')
                *contador += 1;
            if (matriz[i][0] == 'O')
                *contador -= 1;
        }
    }
    pthread_exit(NULL);
}

void *checa_colunas(void *cntdr)
{
    printf("Analisando as colunas\n");
    int *contador = (int *) cntdr;
    for (int i=0; i<3; i++) {
        if (matriz[0][i] == matriz[1][i] && matriz[1][i] == matriz[2][i]) {
            if (matriz[0][i] == 'X')
                *contador += 1;
            if (matriz[0][i] == 'O')
                *contador -= 1;
        }
    }
    pthread_exit(NULL);
}

void *checa_diagonais(void *cntdr)
{
    printf("Analisando as diagonais\n");
    int *contador = (int *) cntdr;
    // '\'
    if (matriz[0][0] == matriz[1][1] && matriz[1][1] == matriz[2][2]) {
        if (matriz[0][0] == 'X')
            *contador += 1;
        if (matriz[0][0] == 'O')
            *contador -= 1;
    }
    // '/'
    if (matriz[0][2] == matriz[1][1] && matriz[1][1] == matriz[2][0]) {
        if (matriz[0][0] == 'X')
            *contador += 1;
        if (matriz[0][0] == 'O')
            *contador -= 1;
    }

    pthread_exit(NULL);
}

int main()
{
    // Lê matriz
    for (int i=0; i<3; i++) {
        scanf(" %3s", matriz[i]);
    }
    printf("Li a matriz.\n");

    // Cria 3 threads: Uma para checar linhas, uma pra colunas e uma pras diagonais, de modo que
    // não ocorra condição de disputa, pois elas estão apenas lendo da matriz e cada uma está 
    // checando coisas diferentes.
    pthread_t linha, coluna, diagonal;
    int res_linha=0, res_coluna=0, res_diagonal=0;
    // Envia o endereço de variáveis contadoras para poder usá-las na verificação
    printf("Criando Threads...\n");
    pthread_create(&linha, NULL, checa_linhas, (void *) &res_linha);
    pthread_create(&coluna, NULL, checa_colunas, (void *) &res_coluna);
    pthread_create(&diagonal, NULL, checa_diagonais, (void *) &res_diagonal);
    printf("Threads Criadas.\n");

    // Espera as threads terminarem
    pthread_join(linha, NULL);
    pthread_join(coluna, NULL);
    pthread_join(diagonal, NULL);

    // Compara os resultados das threads e printa o resultado
    int soma = res_linha + res_coluna + res_diagonal;
    printf("A soma deu %d\n", soma);
    if (soma > 0) {
        printf("O 'X' venceu!\n");
    } else if (soma < 0) {
        printf("O 'O' venceu!\n");
    } else {
        printf("Deu velha.\n");
    }

    pthread_exit(NULL);
}