#include <stdio.h>
#include <stdlib.h>
#include "hash.h"


// QUEBRANDO O TIPO OPACO (APENAS PARA ESTE TESTE)

typedef struct Node_t {
    char* key;
    int value;
    struct Node_t* next;
} Node_t;


typedef struct HashTable_t {
    Node_t** table; 
    int size;       
} HashTable_t;

static unsigned long int hash_djb2(char *chave){
    unsigned long hash = 5381;
    int x;
    while((x=*chave++)){
        hash = ((hash<<5)+ hash) + x;
    }
    return hash;
}

static int balde_index(char*chave, int size){
    return hash_djb2(chave) % size;
}


int main() {
   printf("--- Iniciando Testes para as Funções de Hashing ---\n\n");

  
    const char* chaves_teste[] = {"v10", "v20", "na", "nb", "uma_chave_muito_longa_para_ver_o_que_acontece", "v10", ""};
    int num_chaves = 7;
    int tamanho_tabela = 157; 

    printf("Tamanho da tabela para o teste de índice: %d\n\n", tamanho_tabela);

    for (int i = 0; i < num_chaves; i++) {
        const char* chave_atual = chaves_teste[i];

        // teste da função hash_djb2
        unsigned long hash_calculado = hash_djb2((char*)chave_atual);

        // teste a função balde_index
        int indice_calculado = balde_index((char*)chave_atual, tamanho_tabela);

        printf("Chave: \"%s\"\n", chave_atual);
        printf("  -> Hash DJB2: %lu\n", hash_calculado);
        printf("  -> Índice do Balde: %d\n", indice_calculado);

        // verificação importante: 0 índice está dentro dos limites do array?
        if (indice_calculado < 0 || indice_calculado >= tamanho_tabela) {
            printf("  [FALHOU] O índice %d está fora dos limites da tabela (0 a %d)!\n", indice_calculado, tamanho_tabela - 1);
        } else {
            printf("  [OK] O índice está dentro dos limites da tabela.\n");
        }
        printf("\n");
    }

    printf("\n--- Testes Concluídos ---\n");

    return 0;
}