#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"      
#include "via.h"       
#include "graph.h"      
#include "lista.h"     

int main(int argc, char *argv[]) {
    // -----------------------------------------------------------
    // PASSO 1: LER E VALIDAR OS PARÂMETROS DE ENTRADA
    // -----------------------------------------------------------
    printf("1. Processando parâmetros da linha de comando...\n");
    void* params = criarParametros();
    if (!processarArgumentos(params, argc, argv)) {
        destruirParametros(params);
        return 1; // Termina se os parâmetros forem inválidos
    }
    
    // Pega o caminho do arquivo .via, que é essencial para esta etapa
    char* caminho_via = getCaminhoCompletoVia(params);
    if (caminho_via == NULL) {
        fprintf(stderr, "Erro: O parâmetro -v com o arquivo de vias (.via) é necessário para este teste.\n");
        destruirParametros(params);
        return 1;
    }
    printf("   -> Arquivo .via a ser processado: %s\n", caminho_via);


    // -----------------------------------------------------------
    // PASSO 2: CARREGAR O GRAFO
    // -----------------------------------------------------------
    printf("\n2. Carregando o grafo a partir do arquivo .via...\n");
    
    // Chama a função do módulo 'via' para fazer todo o trabalho pesado
    Graph g = carregarGrafoDeArquivoVia(caminho_via);
    
    if (g == NULL) {
        fprintf(stderr, "   [FALHOU] Erro fatal ao carregar o grafo.\n");
        free(caminho_via);
        destruirParametros(params);
        return 1;
    }
    printf("   [OK] Grafo carregado com sucesso!\n");


    // -----------------------------------------------------------
    // PASSO 3: SESSÃO DE TESTE E VERIFICAÇÃO
    // -----------------------------------------------------------
    printf("\n3. Verificando a integridade do grafo carregado...\n");

    // Teste 1: Verificar o número total de nós
    int nos_lidos = getTotalNodes(g);
    printf("   - Teste de Contagem de Nós: %d vértices foram carregados.\n", nos_lidos);

    // Teste 2: Tenta encontrar um vértice conhecido
    const char* vertice_teste = "beS.SE"; // Mude para um vértice que exista no seu .via
    printf("   - Teste de Busca: Procurando pelo vértice '%s'...\n", vertice_teste);
    Node id_teste = getNode(g, (char*)vertice_teste);
    if (id_teste != -1) {
        printf("     [OK] Vértice '%s' encontrado com ID: %d.\n", vertice_teste, id_teste);

        // Teste 3: Listar as arestas que saem desse vértice
        printf("   - Teste de Adjacência: Listando ruas que saem de '%s' (ID %d)...\n", vertice_teste, id_teste);
        Lista arestas_vizinhas = lista_cria();
        adjacentEdges(g, id_teste, arestas_vizinhas);
        
        int num_vizinhos = lista_tamanho(arestas_vizinhas);
        printf("     -> Encontrado(s) %d segmento(s) de rua saindo deste ponto.\n", num_vizinhos);

        if (num_vizinhos > 0) {
            printf("     -> Destinos: ");
            Iterador it = lista_iterador(arestas_vizinhas);
            while (iterador_tem_proximo(it)) {
                Edge aresta_opaca = (Edge)iterador_proximo(it);
                Node id_destino = getToNode(g, aresta_opaca);
                printf("%d ", id_destino);
            }
         printf("\n");
         iterador_destroi(it);
         lista_libera(arestas_vizinhas);

    } else 
        printf("     [FALHOU] Vértice '%s' não foi encontrado na tabela hash.\n", vertice_teste);
 }

    printf("\n4. Finalizando e liberando toda a memória...\n");
    killDG(g);
    free(caminho_via);
    destruirParametros(params);
    
    printf("   [OK] Memória liberada.\n\n");
    printf("Execução de teste do .via concluída com sucesso!\n");

    return 0;
}