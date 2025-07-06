#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include <stdbool.h>
#include <string.h>

// QUEBRANDO O TIPO OPACO (APENAS PARA ESTE TESTE)
typedef struct Node {
    char* chave;
    int valor;
    struct Node* prox;
} Node;

typedef struct {
    Node** balde;
    int size;
} tabelaHash;


static unsigned long int hash_djb2(char *chave){
    unsigned long hash = 5381;
    int x;
    while((x=*chave++)){
        hash = ((hash<<5)+ hash) + x;
    }
    return hash;
}

int balde_index(char*chave, int size){
    return hash_djb2(chave) % size;
}
int main() {
    printf("--- Teste Completo do Módulo de Tabela Hash ---\n\n");

    hashTable ht = createHashTable(13);
    printf("[INFO] Tabela criada.\n");

    // --- Testando hashPut ---
    printf("\n--- Inserindo elementos ---\n");
    hashPut(ht, "v10", 10);
    hashPut(ht, "v20", 20);
    hashPut(ht, "ac", 100); // Colide com "p"
    hashPut(ht, "p", 200);  // Colide com "ac"
    printf("Inseridos: ('v10', 10), ('v20', 20), ('ac', 100), ('p', 200)\n");

    // --- Testando hashGet ---
    printf("\n--- Verificando com hashGet ---\n");
    int valor;
    if (hashGet(ht, "v20", &valor) && valor == 20) {
        printf("  [OK] Chave 'v20' encontrada com valor %d.\n", valor);
    } else {
        printf("  [FALHOU] Erro ao buscar 'v20'.\n");
    }

    if (hashGet(ht, "p", &valor) && valor == 200) {
        printf("  [OK] Chave 'p' (com colisão) encontrada com valor %d.\n", valor);
    } else {
        printf("  [FALHOU] Erro ao buscar 'p'.\n");
    }

    if (!hashGet(ht, "chave_inexistente", &valor)) {
        printf("  [OK] Chave 'chave_inexistente' não foi encontrada, como esperado.\n");
    } else {
        printf("  [FALHOU] Chave 'chave_inexistente' foi encontrada incorretamente.\n");
    }

    // --- Testando hashRemove ---
    printf("\n--- Verificando com hashRemove ---\n");
    printf("Removendo 'p' (primeiro nó de uma lista com colisão)...\n");
    hashRemove(ht, "p");

    if (!hashGet(ht, "p", &valor)) {
        printf("  [OK] Chave 'p' não foi mais encontrada.\n");
    } else {
        printf("  [FALHOU] Chave 'p' ainda existe após remoção.\n");
    }

    if (hashGet(ht, "ac", &valor) && valor == 100) {
        printf("  [OK] Chave 'ac' (que estava na mesma lista) ainda existe.\n");
    } else {
        printf("  [FALHOU] Chave 'ac' foi afetada incorretamente pela remoção de 'p'.\n");
    }

    // --- Testando hashTableDestroy ---
    printf("\n--- Verificando com hashDestroy ---\n");
    hashTableDestroy(ht);
    printf("  [OK] Tabela destruída. Use Valgrind para garantir que não há vazamento de memória.\n");


    printf("\n--- Testes Concluídos ---");
    return 0;
}


