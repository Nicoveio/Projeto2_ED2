#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include "qry.h"
#include "graph.h"
#include "lista.h"
#include "hash.h"
#include "utils.h"
#include "svg.h"
#include "geo.h" 
#include "via.h"

#define INFINITO DBL_MAX

typedef struct {
    char cep[50];
    double x, y, w, h;
    char cfill[25], cstrk[25], sw[8];
} Quadra;

typedef struct {
    double x;
    double y;
    bool definido; 
} CoordenadasReg;

typedef enum {
    TIPO_MARCADOR_O,
    TIPO_CAMINHO,
    TIPO_RETANGULO_ALAGAMENTO

} TipoElementoVisual;

typedef struct {
    char nome[50];
    Lista caminho_curto;
    Lista caminho_rapido;
    bool acessivel;
} Percurso;

typedef struct {
    char registrador[8];
    double x;
    double y;
    char face;
} MarcadorO;

typedef struct {
    TipoElementoVisual tipo;
    void* dados; 
} ElementoVisual;

typedef struct {
    Lista elementos_para_desenhar;
} ResultadosConsultaImp;

typedef struct {
    char* nome_rua;
    char* cep_direita;
    char* cep_esquerda;
    double comprimento;
    double velocidade_media;
} InfoAresta;

static Quadra* encontrarQuadraPorCep(Lista lista_de_quadras, const char* cep) {
    if (!lista_de_quadras || !cep) return NULL;

    Iterador it = lista_iterador(lista_de_quadras);
    while (iterador_tem_proximo(it)) {
        Quadra* q = (Quadra*)iterador_proximo(it);
        if (strcmp(q->cep, cep) == 0) {
            iterador_destroi(it);
            return q; 
        }
    }
    iterador_destroi(it);
    return NULL; 
}


static CoordenadasReg calcularCoordenada(Quadra* q, char face, double num) {
    CoordenadasReg c = {0, 0, false};
    if (!q) return c;
    c.definido = true;
    
    // Bússola do projeto: N=Baixo, S=Cima, L=Esquerda, O=Direita
    switch (face) {
        case 'N': // Norte (borda de BAIXO) -> Face Horizontal
        case 'n':
            c.x = q->x + num;     // Varia em X
            c.y = q->y + q->h;    // Y é fixo (na borda de baixo)
            break;

        case 'S': // Sul (borda de CIMA) -> Face Horizontal
        case 's':
            c.x = q->x + num;     // Varia em X
            c.y = q->y;           // Y é fixo (na borda de cima)
            break;

        case 'L': // Leste (borda da ESQUERDA) -> Face Vertical
        case 'l':
            c.x = q->x;           // X é fixo
            c.y = q->y + num;     // Varia em Y
            break;

        case 'O': // Oeste (borda da DIREITA) -> Face Vertical
        case 'o':
            c.x = q->x + q->w;    // X é fixo
            c.y = q->y + num;     // Varia em Y
            break;
    }
    return c;
}


static void executaArrobaO(const char* linha, Lista quadras, CoordenadasReg* regs, Lista elementos_visuais, FILE* txt) {
    char reg_str[4], cep[50], face_str[2];
    double num;
    sscanf(linha, "@o? %s %s %s %lf", reg_str, cep, face_str, &num);
    
    fprintf(txt, "[@o?*]\n");
    fprintf(txt, "\tCEP:%s Face:%s Num:%.2lf\n", cep, face_str, num);

    Quadra* q_alvo = encontrarQuadraPorCep(quadras, cep);
    if (q_alvo) {
        CoordenadasReg coord = calcularCoordenada(q_alvo, face_str[0], num);
        int indice_reg = atoi(reg_str + 1);
        if (indice_reg >= 0 && indice_reg <= 10) {
            regs[indice_reg] = coord;
            fprintf(txt, "\tRegistrador %s -> Coordenada (%.2f, %.2f)\n", reg_str, coord.x, coord.y);
            MarcadorO* marcador = malloc(sizeof(MarcadorO));
            strcpy(marcador->registrador, reg_str);
            marcador->x = coord.x;
            marcador->y = coord.y;
            marcador->face = face_str[0];

            ElementoVisual* el_visual = malloc(sizeof(ElementoVisual));
            el_visual->tipo = TIPO_MARCADOR_O;
            el_visual->dados = marcador;
            lista_insere(elementos_visuais, el_visual);
        }
    } else {
        fprintf(txt, "\tERRO: CEP '%s' não encontrado.\n", cep);
    }
}

static const char* obterDirecao(CoordenadasReg* origem, CoordenadasReg* destino) {
    if (!origem || !destino) {
        return "desconhecida";
    }
    double dx = destino->x - origem->x;
    double dy = destino->y - origem->y;

    if (fabs(dx) > fabs(dy)) {
        if (dx > 0) {
            return "leste";
        } else {
            return "oeste";
        }
    } else {
        if (dy > 0) {
            return "norte";
        } else {
            return "sul";
        }
    }
}


static void escreverRelatorioPercurso(FILE* txt, Graph g, Lista caminho) {
    if (lista_vazia(caminho)) {
        fprintf(txt, "\t-> ATENÇÃO: Destino inacessível a partir da origem.\n");
        return;
    }

    fprintf(txt, "\t- Percurso Textual Detalhado -\n");
    
    Node no_anterior = -1;
    Iterador it = lista_iterador(caminho);
    
    if (iterador_tem_proximo(it)) {
        no_anterior = (Node)(uintptr_t)iterador_proximo(it);
        fprintf(txt, "\tPonto de partida no cruzamento: %s\n", getNodeName(g, no_anterior));
    }

    while (iterador_tem_proximo(it)) {
        Node no_atual = (Node)(uintptr_t)iterador_proximo(it);
        Edge aresta_percorrida = getEdge(g, no_anterior, no_atual);
        
        if (aresta_percorrida) {
            InfoAresta* info = (InfoAresta*)getEdgeInfo(g, aresta_percorrida);
            CoordenadasReg* coord_anterior = (CoordenadasReg*)getNodeInfo(g, no_anterior);
            CoordenadasReg* coord_atual = (CoordenadasReg*)getNodeInfo(g, no_atual);
            const char* direcao = obterDirecao(coord_anterior, coord_atual);

            fprintf(txt, "\tSiga na direção %s na rua \"%s\" até o cruzamento %s\n", 
                    direcao, info->nome_rua, getNodeName(g, no_atual));
        }
        
        no_anterior = no_atual;
    }
    
    fprintf(txt, "\tVocê chegou ao seu destino no cruzamento: %s\n", getNodeName(g, no_anterior));
    iterador_destroi(it);
}
static void executaPInterrogacao(const char* linha, Graph g, CoordenadasReg* regs, hashTable percursos, FILE* txt, CalculaCustoAresta funcCusto) {
    char np[50], nome_subgrafo[50], reg1_str[4], reg2_str[4];
    sscanf(linha, "p? %s %s %s %s", np, nome_subgrafo, reg1_str, reg2_str);

    fprintf(txt, "[p?*]\n\tPercurso: %s, Grafo: %s, Origem: %s, Destino: %s\n", np, nome_subgrafo, reg1_str, reg2_str);

    int idx1 = atoi(reg1_str + 1);
    int idx2 = atoi(reg2_str + 1);

    if (!regs[idx1].definido || !regs[idx2].definido) {
        fprintf(txt, "\tERRO: Registrador %s ou %s não foi definido pelo comando @o?.\n", reg1_str, reg2_str);
        return;
    }

    CoordenadasReg coord_inicio = regs[idx1];
    CoordenadasReg coord_fim = regs[idx2];
    
    Node no_inicio = findNearestNode(g, coord_inicio.x, coord_inicio.y);
    Node no_fim = findNearestNode(g, coord_fim.x, coord_fim.y);

    if (no_inicio == -1 || no_fim == -1) {
        fprintf(txt, "\tERRO: Não foi possível encontrar vértices próximos aos endereços definidos.\n");
        return;
    }

    fprintf(txt, "\tMapeado para Vértices -> Origem: %s (ID %d), Destino: %s (ID %d)\n",
            getNodeName(g, no_inicio), no_inicio, getNodeName(g, no_fim), no_fim);

    Lista caminho_curto = findPath(g, no_inicio, no_fim, CRITERIO_DISTANCIA, funcCusto);
    Lista caminho_rapido = findPath(g, no_inicio, no_fim, CRITERIO_TEMPO, funcCusto);

    Percurso* novo_percurso = malloc(sizeof(Percurso));
    strcpy(novo_percurso->nome, np);
    novo_percurso->caminho_curto = caminho_curto;
    novo_percurso->caminho_rapido = caminho_rapido;
    novo_percurso->acessivel = !lista_vazia(caminho_curto);

    hashPut(percursos, np, (int)(uintptr_t)novo_percurso);

    if (novo_percurso->acessivel) {
        fprintf(txt, "\t-> Caminho encontrado! Detalhes salvos no percurso '%s'.\n", np);
        escreverRelatorioPercurso(txt, g, novo_percurso->caminho_curto);
    } else {
        fprintf(txt, "\t-> ATENÇÃO: Destino inacessível a partir da origem.\n");
    }
}
ResultadosConsulta processaQry(Graph g, Lista quadras, const char* caminho_qry, const char* caminho_txt_saida, CalculaCustoAresta funcCusto) {
    FILE* arquivo_qry = fopen(caminho_qry, "r");
    FILE* arquivo_txt = fopen(caminho_txt_saida, "w");
    if (!arquivo_qry || !arquivo_txt) {
        fprintf(stderr, "ERRO: Não foi possível abrir os arquivos de consulta ou de saída de texto.\n");
        if (arquivo_qry) fclose(arquivo_qry);
        if (arquivo_txt) fclose(arquivo_txt);
        return NULL;
    }


    CoordenadasReg registradores[11];
    for (int i = 0; i < 11; i++) registradores[i].definido = false;

    ResultadosConsultaImp* resultados = malloc(sizeof(ResultadosConsultaImp));
    hashTable tabela_de_percursos = createHashTable(29);
    resultados->elementos_para_desenhar = lista_cria();
    
    char buffer_linha[1024];
    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo_qry) != NULL) {
        buffer_linha[strcspn(buffer_linha, "\r\n")] = '\0';
        if (buffer_linha[0] == '\0' || buffer_linha[0] == '#') continue;

        //GUIA QUE MONTEI COM QRY DO QUE FAZER EM CADA FUNÇÃO

        fprintf(arquivo_txt, "Comando: %s\n", buffer_linha);

        // COMANDO @o? 
        if (strncmp(buffer_linha, "@o?", 3) == 0) {
             executaArrobaO(buffer_linha, quadras, registradores, resultados->elementos_para_desenhar, arquivo_txt);
        
        //  COMANDO alag
        } else if (strncmp(buffer_linha, "alag", 4) == 0) {
            printf("processa alag simulação");
            fprintf(arquivo_txt, "  -> Comando 'alag' identificado, mas não implementado.\n");

            //'Parsear' n, x, y, w, h da linha.
            //Usar a SmuTreap (getNodeSmusDentroRegiaoSmuT?) para encontrar os vértices na área.
            //Para cada vértice encontrado, percorrer sua lista de adjacência e chamar disableEdge.
            //Guardar as arestas desabilitadas associadas ao 'n'.
            //Adicionar o retângulo de alagamento à 'elementos_para_desenhar'.
            //Escrever as arestas desabilitadas no .txt.

        // COMANDO dren 
        } else if (strncmp(buffer_linha, "dren", 4) == 0) {
            printf("processa dren simulação");
            fprintf(arquivo_txt, "  -> Comando 'dren' identificado, mas não implementado.\n");
            //Parsear 'n' da linha.
            //Recuperar a lista de arestas associadas ao 'n' do alagamento.
            //Chamar enableEdge para cada uma.
            //Escrever as arestas reabilitadas no .txt.

        //  COMANDO sg 
        } else if (strncmp(buffer_linha, "sg", 2) == 0) {
            printf("processa sg simulação");
            fprintf(arquivo_txt, "  -> Comando 'sg' identificado, mas não implementado.\n");
            //Parsear nome, x, y, w, h da linha.
            //Usar a SmuTreap para encontrar os vértices na área?
            //Chamar createSubgraphDG do graph.c.
            //Adicionar o retângulo pontilhado à 'elementos_para_desenhar'.

        // COMANDO p? 
        } else if (strncmp(buffer_linha, "p?", 2) == 0) {
            executaPInterrogacao(buffer_linha, g, registradores, tabela_de_percursos, arquivo_txt, funcCusto);
        } else if (strncmp(buffer_linha, "join", 4) == 0) {
            printf("processa join simulação");
            fprintf(arquivo_txt, "  -> Comando 'join' identificado, mas não implementado.\n");
            //1. Semelhante a p?

        // COMANDO shw 
        } else if (strncmp(buffer_linha, "shw", 3) == 0) {
            printf("processa shw simulação");
            fprintf(arquivo_txt, "  -> Comando 'shw' identificado, mas não implementado.\n");
            // Parsear np, cmc, cmr.
            // Buscar o Percurso na 'tabela_de_percursos' pela chave 'np'.
            // Adicionar o percurso e suas cores à lista 'elementos_para_desenhar'.
        }

        fprintf(arquivo_txt, "\n");
    }

    printf("Processamento de consultas concluído. Gerando SVG final...\n");

    hashTableDestroy(tabela_de_percursos);

    fclose(arquivo_qry);
    fclose(arquivo_txt);
    return (ResultadosConsulta)resultados;
}

void liberaResultadosConsulta(ResultadosConsulta res) {
    if (!res) return;
    ResultadosConsultaImp* resultados = (ResultadosConsultaImp*)res;
    
    Iterador it = lista_iterador(resultados->elementos_para_desenhar);
    while(iterador_tem_proximo(it)) {
        ElementoVisual* el = (ElementoVisual*)iterador_proximo(it);
        free(el->dados); 
        free(el);      
    }
    iterador_destroi(it);
    lista_libera(resultados->elementos_para_desenhar);
    free(resultados);
}


Lista getElementosParaDesenhar(ResultadosConsulta res) {
    if (!res) return NULL;
    ResultadosConsultaImp* resultados = (ResultadosConsultaImp*)res;
    return resultados->elementos_para_desenhar;
}