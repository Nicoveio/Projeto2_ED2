#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include <stdbool.h>
#include <string.h>
#include "priority_queue.h"

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
    printf("--- Iniciando Testes para o Módulo de Fila de Prioridade ---\n\n");

    // --- Teste 1: Operações Básicas ---
    printf("--- Teste 1: Testando criação e estado de fila vazia ---\n");
    priorityQueue pq = createPriorityQueue(10);

    if (pq == NULL) {
        printf("  [FALHOU] createPriorityQueue() retornou NULL.\n");
        return 1;
    }
    printf("  [OK] Fila criada com sucesso.\n");

    if (pq_empty(pq)) {
        printf("  [OK] pq_empty() retornou 'true' para uma fila nova.\n");
    } else {
        printf("  [FALHOU] pq_empty() deveria retornar 'true' para uma fila nova.\n");
    }

    printf("Inserindo o item 100 com prioridade 10.0...\n");
    pq_insert(pq, 100, 10.0);

    if (!pq_empty(pq)) {
        printf("  [OK] pq_empty() retornou 'false' após uma inserção.\n");
    } else {
        printf("  [FALHOU] pq_empty() deveria retornar 'false' após uma inserção.\n");
    }
    printf("\n");

    // --- Teste 2: Ordem de Prioridade ---
    printf("--- Teste 2: Verificando a ordem de extração (o teste principal) ---\n");
    // Inserindo itens com prioridades fora de ordem
    printf("Inserindo múltiplos itens com prioridades fora de ordem...\n");
    pq_insert(pq, 20, 2.5);  // Maior prioridade
    pq_insert(pq, 30, 15.0); // Menor prioridade
    pq_insert(pq, 40, 5.0);  // Segunda maior prioridade

    // A ordem esperada de extração é: 20 (p=2.5), 40 (p=5.0), 100 (p=10.0), 30 (p=15.0)
    int ordem_esperada[] = {20, 40, 100, 30};
    int itens_extraidos[4];
    int i = 0;

    printf("Extraindo todos os itens para verificar a ordem...\n");
    while (!pq_empty(pq)) {
        itens_extraidos[i] = pq_extract_min(pq);
        printf("  Extraído: %d\n", itens_extraidos[i]);
        i++;
    }

    // Compara a ordem extraída com a esperada
    bool ordem_correta = true;
    for (i = 0; i < 4; i++) {
        if (itens_extraidos[i] != ordem_esperada[i]) {
            ordem_correta = false;
            break;
        }
    }

    if (ordem_correta) {
        printf("  [OK] Itens foram extraídos na ordem correta de prioridade.\n");
    } else {
        printf("  [FALHOU] A ordem de extração está incorreta.\n");
    }

    if (pq_empty(pq)) {
        printf("  [OK] Fila está vazia após todas as extrações.\n");
    } else {
        printf("  [FALHOU] Fila deveria estar vazia.\n");
    }
    printf("\n");

    // --- Teste 3: Casos Especiais ---
    printf("--- Teste 3: Extração de fila vazia ---\n");
    int item_erro = pq_extract_min(pq);
    if (item_erro == -1) {
        printf("  [OK] pq_extract_min() em fila vazia retornou -1, como esperado.\n");
    } else {
        printf("  [FALHOU] pq_extract_min() em fila vazia deveria retornar -1.\n");
    }

    // --- Limpeza Final ---
    pq_destroy(pq);
    printf("\n[INFO] Teste de destruição executado. Use Valgrind para checar vazamentos.\n");
    printf("\n--- Testes Concluídos ---\n");

    return 0;
}

