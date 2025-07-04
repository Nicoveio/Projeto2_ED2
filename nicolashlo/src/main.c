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



int main() {
    printf("--- Iniciando Testes para o Módulo de Tabela Hash ---\n\n");


    printf("Teste 1: Tentando criar tabela com tamanho 157...\n");
    int tamanho_teste = 157;
    hashTable ht = createHashTable(tamanho_teste);

    if (ht == NULL) {
        printf("  [FALHOU] createHashTable() retornou NULL para um tamanho válido.\n");
        return 1; // Termina o programa com erro
    }
    printf("  [OK] createHashTable() retornou um ponteiro válido.\n");

    
    HashTable_t* ht_interno = (HashTable_t*)ht;

 
    if (ht_interno->size != tamanho_teste) {
        printf("  [FALHOU] Tamanho armazenado na tabela (%d) é diferente do esperado (%d).\n", ht_interno->size, tamanho_teste);
    } else {
        printf("  [OK] Tamanho da tabela foi armazenado corretamente.\n");
    }

 
    if (ht_interno->table == NULL) {
        printf("  [FALHOU] O array de buckets (table) está nulo.\n");
    } else {
        printf("  [OK] O array de buckets (table) foi alocado.\n");

        int i;
        for (i = 0; i < tamanho_teste; i++) {
            if (ht_interno->table[i] != NULL) {
                printf("  [FALHOU] O bucket %d não foi inicializado como NULL.\n", i);
                break; 
            }
        }
        if (i == tamanho_teste) {
            printf("  [OK] Todos os %d buckets foram inicializados como NULL.\n", tamanho_teste);
        }
    }

  

    // Teste com tamanho zero
    ht = createHashTable(0);
    if (ht == NULL) {
        printf("  [OK] createHashTable(0) retornou NULL como esperado.\n");
    } else {
        printf("  [FALHOU] createHashTable(0) deveria retornar NULL.\n");
    }

    // Teste com tamanho negativo
    ht = createHashTable(-10);
    if (ht == NULL) {
        printf("  [OK] createHashTable(-10) retornou NULL como esperado.\n");
    } else {
        printf("  [FALHOU] createHashTable(-10) deveria retornar NULL.\n");
    }

    printf("\n--- Testes Concluídos ---\n");

    return 0;
}