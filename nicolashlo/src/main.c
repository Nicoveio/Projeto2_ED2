#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include <stdbool.h>
#include <string.h>
#include "priority_queue.h"
#include "graph.h"
#include "lista.h"
#include <math.h>
#include <stdint.h>
#include <float.h>

#define INFINITO DBL_MAX





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

typedef struct {
    char* nome_rua;
    double comprimento;
    double velocidade_media;
} InfoAresta;

double minhaFuncaoDeCusto(Info info_aresta, int criterio) {
    if (!info_aresta) return INFINITO;
    InfoAresta* info = (InfoAresta*)info_aresta;
    
    if (criterio == CRITERIO_DISTANCIA) {
        return info->comprimento;
    } else { // CRITERIO_TEMPO
        if (info->velocidade_media > 0) {
            return info->comprimento / info->velocidade_media;
        }
        return INFINITO;
    }
}

typedef struct{
    double x;
    double y;
}Coordenadas;


int main() {
  printf("--- Iniciando Teste Completo do Módulo Grafo ---\n\n");

    // --- 1. Teste de Criação do Grafo ---
    printf("--> PASSO 1: Criando o grafo para uma cidade com 6 cruzamentos...\n");
    int num_vertices = 6;
    Graph g = createGraph(num_vertices, true, "Bitnopolis_Teste");
    if (!g) {
        printf("  [FALHOU] createGraph retornou NULL.\n");
        return 1;
    }
    printf("  [OK] Grafo criado com sucesso.\n\n");


    // --- 2. Teste de Adição de Vértices (addNode) ---
    printf("--> PASSO 2: Adicionando 6 vértices (cruzamentos)...\n");
    const char* nomes_v[] = {"v1", "v2", "v3", "v4", "v5", "v6"};
    double coords_v[][2] = {{0,0}, {100,0}, {200,0}, {0,100}, {100,100}, {200,100}};
    Node ids[num_vertices];

    for (int i = 0; i < num_vertices; i++) {
        // O main.c aloca a struct de Coordenadas
        Coordenadas* c = malloc(sizeof(Coordenadas));
        c->x = coords_v[i][0];
        c->y = coords_v[i][1];
        // e a passa para o grafo como um Info (void*)
        ids[i] = addNode(g, (char*)nomes_v[i], c);
        printf("  - addNode('%s') -> ID: %d\n", nomes_v[i], ids[i]);
    }
    printf("  [OK] Vértices adicionados. Total de nós no grafo: %d\n\n", getTotalNodes(g));


    // --- 3. Teste de Adição de Arestas (addEdge) ---
    printf("--> PASSO 3: Adicionando 7 arestas (ruas)...\n");
    // (de, para, comprimento, velocidade)
    double arestas_info[][4] = { {0,1, 100, 10}, {1,2, 100, 10}, {0,3, 100, 5},
                                 {1,4, 100, 10}, {2,5, 100, 5}, {3,4, 100, 10}, {4,5, 100, 10} };

    for (int i = 0; i < 7; i++) {
        // O main.c aloca a struct InfoAresta
        InfoAresta* info_a = malloc(sizeof(InfoAresta));
        info_a->nome_rua = "Rua Teste"; // Apenas um nome genérico para o teste
        info_a->comprimento = arestas_info[i][2];
        info_a->velocidade_media = arestas_info[i][3];
        
        Node from_id = (Node)arestas_info[i][0];
        Node to_id = (Node)arestas_info[i][1];
        // e a passa para o grafo como um Info (void*)
        addEdge(g, from_id, to_id, info_a);
        printf("  - addEdge(%d -> %d) adicionada.\n", from_id, to_id);
    }
    printf("  [OK] Arestas adicionadas.\n\n");
    

    // --- 4. Teste de Busca de Caminho (findPath) ---
    printf("--> PASSO 4: Testando findPath de v1 (ID 0) para v6 (ID 5)...\n");
    Node inicio = ids[0];
    Node fim = ids[5];

    // a) Por Distância
    printf("  - Buscando pelo caminho MAIS CURTO (distância)...\n");
    // O main.c passa a "ferramenta" minhaFuncaoDeCusto para o grafo
    Lista caminho_dist = findPath(g, inicio, fim, CRITERIO_DISTANCIA, minhaFuncaoDeCusto);
    
    printf("    Caminho encontrado: ");
    Iterador it_dist = lista_iterador(caminho_dist);
    while (iterador_tem_proximo(it_dist)) {
        Node no_id = (Node)(uintptr_t)iterador_proximo(it_dist);
        printf("%d ", no_id);
    }
    printf("\n");
    iterador_destroi(it_dist);
    // IMPORTANTE: O findPath retorna uma lista de IDs. A lista em si pode ser liberada.
    lista_libera(caminho_dist);

    // b) Por Tempo
    printf("  - Buscando pelo caminho MAIS RÁPIDO (tempo)...\n");
    Lista caminho_tempo = findPath(g, inicio, fim, CRITERIO_TEMPO, minhaFuncaoDeCusto);
    
    printf("    Caminho encontrado: ");
    Iterador it_tempo = lista_iterador(caminho_tempo);
    while (iterador_tem_proximo(it_tempo)) {
        Node no_id = (Node)(uintptr_t)iterador_proximo(it_tempo);
        printf("%d ", no_id);
    }
    printf("\n  [OK] Testes de findPath concluídos.\n\n");
    iterador_destroi(it_tempo);
    lista_libera(caminho_tempo);


    // --- 5. Teste de Destruição do Grafo ---
    killDG(g); 
    printf("  [OK] Grafo destruído.\n\n");
    
    printf("--- Testes Concluídos ---\n");
    return 0;
}

