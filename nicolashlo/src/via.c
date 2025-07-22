
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "via.h"
#include "graph.h"
#include "utils.h" 


typedef struct {
    double x;
    double y;
} Coordenadas;

typedef struct {
    char* nome_rua;
    char* cep_direita;
    char* cep_esquerda;
    double comprimento;
    double velocidade_media;
} InfoAresta;


// Implementação da função principal do módulo
Graph carregarGrafoDeArquivoVia(const char* caminho_via) {
    FILE* arquivo = fopen(caminho_via, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo .via em '%s'\n", caminho_via);
        return NULL;
    }

    // lê o número de vértices (nv) da primeira linha
    int nVert;
    if (fscanf(arquivo, "%d", &nVert) != 1) {
        fprintf(stderr, "Erro: falha ao ler o número de vértices do arquivo .via\n");
        fclose(arquivo);
        return NULL;
    }
    
    // consome o resto da linha (o caractere '\n')
    fgetc(arquivo);

    //cria a estrutura do grafo com o tamanho correto
    Graph g = createGraph(nVert, true, "Bitnopolis");
    if (g == NULL) {
        fprintf(stderr, "Erro: não foi possível criar o grafo.\n");
        fclose(arquivo);
        return NULL;
    }

    char buffer_linha[1024]; // Um buffer para ler cada linha

    // lê o resto do arquivo linha por linha
    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo) != NULL) {
        
        // se a linha começar com 'v' (vértice)
if (buffer_linha[0] == 'v') {
    char tipo, nome_vertice[100];
    double x, y;

    if (sscanf(buffer_linha, "%c %s %lf %lf", &tipo, nome_vertice, &x, &y) == 4) {
        
        // --- ADICIONE ESTA LINHA ---
        printf("DEBUG [via.c]: Guardando a chave: '[%s]'\n", nome_vertice);
        // ---------------------------

        Coordenadas* c = malloc(sizeof(Coordenadas));
        c->x = x;
        c->y = y;
        addNode(g, nome_vertice, (Info)c);
    }
} else if (buffer_linha[0] == 'e') {
            char nome_origem[100], nome_destino[100], cep_esq[100], cep_dir[100], nome_rua[256];
            double cmp, vm;
            
            // O formato "%[^\n]" lê tudo até o final da linha, para pegar nomes de rua com espaços
            sscanf(buffer_linha, "e %s %s %s %s %lf %lf %[^\n]",
                   nome_origem, nome_destino, cep_esq, cep_dir, &cmp, &vm, nome_rua);
            
            Node id_origem = getNode(g, nome_origem);
            Node id_destino = getNode(g, nome_destino);

            if (id_origem != -1 && id_destino != -1) {
                InfoAresta* info_a = malloc(sizeof(InfoAresta));
                info_a->nome_rua = duplicar_string(nome_rua);
                info_a->cep_direita = duplicar_string(cep_dir);
                info_a->cep_esquerda = duplicar_string(cep_esq);
                info_a->comprimento = cmp;
                info_a->velocidade_media = vm;

                addEdge(g, id_origem, id_destino, (Info)info_a);
            }
        }
    }

    fclose(arquivo);
    return g;
}