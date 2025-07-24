#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "via.h"
#include "graph.h"
#include "utils.h"
#include "float.h"

// Structs da cidade
typedef struct { double x; double y; } Coordenadas;
typedef struct {
    char* nome_rua;
    char* cep_direita;
    char* cep_esquerda;
    double comprimento;
    double velocidade_media;
} InfoAresta;

double calculaCustoAresta(Info info_aresta, int criterio) {
    if (!info_aresta) return DBL_MAX; // DBL_MAX é nosso INFINITO

    InfoAresta* info = (InfoAresta*)info_aresta;
    
    if (criterio == CRITERIO_DISTANCIA) {
        return info->comprimento;
    } else { // CRITERIO_TEMPO
        if (info->velocidade_media > 0) {
            // Tempo = Distância / Velocidade
            return info->comprimento / info->velocidade_media;
        }
        return DBL_MAX; 
    }
}
Graph carregarGrafoDeArquivoVia(const char* caminho_via) {
    FILE* arquivo = fopen(caminho_via, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo .via em '%s'\n", caminho_via);
        return NULL;
    }

    int nVert;
    fscanf(arquivo, "%d", &nVert);
    fgetc(arquivo); // Consome o '\n' após o número

    Graph g = createGraph(nVert, true, "Bitnopolis");
    if (g == NULL) {
        fclose(arquivo);
        return NULL;
    }

    char buffer_linha[1024];
    int arestas_adicionadas = 0;

    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo) != NULL) {
        // Remove quebra de linha do final
        buffer_linha[strcspn(buffer_linha, "\r\n")] = '\0';

        if (buffer_linha[0] == 'v') {
            char nome_vertice[100];
            double x, y;
            if (sscanf(buffer_linha, "v %s %lf %lf", nome_vertice, &x, &y) == 3) {
                Coordenadas* c = malloc(sizeof(Coordenadas));
                c->x = x; c->y = y;
                addNode(g, nome_vertice, (Info)c);
            }
        } else if (buffer_linha[0] == 'e') {
            char nome_origem[100], nome_destino[100], cep_esq[100], cep_dir[100];
            double cmp, vm;
            char nome_rua[512]=""; // Buffer para o nome da rua

            // Usa sscanf com um formato que captura o resto da linha
            int itens_lidos = sscanf(buffer_linha, "e %s %s %s %s %lf %lf %[^\n]",
                                     nome_origem, nome_destino, cep_esq, cep_dir, &cmp, &vm, nome_rua);

            if (itens_lidos >= 6) { // >= 6 porque o nome da rua é opcional
                Node id_origem = getNode(g, nome_origem);
                Node id_destino = getNode(g, nome_destino);

                if (id_origem != -1 && id_destino != -1) {
                    InfoAresta* info_a = malloc(sizeof(InfoAresta));
                    info_a->nome_rua = duplicar_string(itens_lidos == 7 ? nome_rua : "");
                    info_a->cep_direita = duplicar_string(cep_dir);
                    info_a->cep_esquerda = duplicar_string(cep_esq);
                    info_a->comprimento = cmp;
                    info_a->velocidade_media = vm;
                    addEdge(g, id_origem, id_destino, (Info)info_a);
                    arestas_adicionadas++;
                } else {
                     printf("  [AVISO] Aresta ignorada: Vértice(s) não encontrado(s). Origem: '%s' (ID:%d), Destino: '%s' (ID:%d)\n", 
                           nome_origem, id_origem, nome_destino, id_destino);
                }
            }
        }
    }

    printf("   [INFO] Total de arestas adicionadas ao grafo: %d\n", arestas_adicionadas);
    fclose(arquivo);
    return g;
}