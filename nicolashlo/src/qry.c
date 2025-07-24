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
#include "smutreap.h"

#define INFINITO DBL_MAX
#define MAX_PERCURSOS 300

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
    TIPO_LINHA_INACESSIVEL,
    TIPO_RETANGULO_ALAGAMENTO

} TipoElementoVisual;

typedef struct {
    char nome[50];
    Lista caminho_curto;
    Lista caminho_rapido;
    bool acessivel;
    Node no_inicio;
    Node no_fim;
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
    Percurso** vestiario_de_percursos; 
    int contador_de_fichas;
    Lista vestiario_de_alagamentos;  
} ResultadosConsultaImp;

typedef struct {
    char* nome_rua;
    char* cep_direita;
    char* cep_esquerda;
    double comprimento;
    double velocidade_media;
} InfoAresta;

typedef struct {
    Lista caminho; 
    char cor[25];  
    bool eh_curto; 
    double duracao_animacao;
} CaminhoVisual;

typedef struct {
    Node no_inicio;
    Node no_fim;
} LinhaInacessivel;

typedef struct {
    double x, y, w, h;
    char cor_borda[25];
    char cor_fill[25];
    char opacidade[8];
} RetanguloVisual;

typedef struct {
    int id;
    Lista arestas_desabilitadas;
} InfoAlagamento;

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
static void executaPInterrogacao(const char* linha, Graph g, CoordenadasReg* regs, hashTable percursos, FILE* txt, CalculaCustoAresta funcCusto, Percurso** vestiario,
 int* contador_fichas) {
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
    novo_percurso->no_inicio = no_inicio;
    novo_percurso->no_fim = no_fim;

    int id_do_percurso = *contador_fichas;
    vestiario[id_do_percurso] = novo_percurso;
    hashPut(percursos, np, id_do_percurso);
    (*contador_fichas)++;


    if (novo_percurso->acessivel) {
        fprintf(txt, "\t-> Caminho encontrado! Detalhes salvos no percurso '%s'.\n", np);
        escreverRelatorioPercurso(txt, g, novo_percurso->caminho_curto);
    } else {
        fprintf(txt, "\t-> ATENÇÃO: Destino inacessível a partir da origem.\n");
    }
}

static double calcularCustoTotalPercurso(Graph g, Lista caminho, int criterio, CalculaCustoAresta funcCusto) {
    if (lista_vazia(caminho)) {
        return 0.0;
    }

    double custo_total = 0.0;
    Node no_anterior = -1;
    Iterador it = lista_iterador(caminho);

    if (iterador_tem_proximo(it)) {
        no_anterior = (Node)(uintptr_t)iterador_proximo(it);
    }

    while (iterador_tem_proximo(it)) {
        Node no_atual = (Node)(uintptr_t)iterador_proximo(it);
        Edge aresta = getEdge(g, no_anterior, no_atual);
        if (aresta) {
            custo_total += funcCusto(getEdgeInfo(g, aresta), criterio);
        }
        no_anterior = no_atual;
    }

    iterador_destroi(it);
    return custo_total;
}
static void executaShw(const char* linha, Graph g, Percurso** vestiario, hashTable percursos, Lista elementos_visuais, FILE* txt, CalculaCustoAresta funcCusto) {
    char np[50], cmc[25], cmr[25];
    sscanf(linha, "shw %s %s %s", np, cmc, cmr);

    fprintf(txt, "[shw*]\n\tExibindo percurso '%s' com cores: Curto='%s', Rápido='%s'\n", np, cmc, cmr);
    
    int id_do_percurso;
    if (hashGet(percursos, np, &id_do_percurso)) {
        printf("DEBUG: Percurso '%s' ENCONTRADO na tabela hash.\n", np); 
        Percurso* p = vestiario[id_do_percurso];


        if (!p->acessivel) {
            printf("DEBUG: Percurso é INACESSÍVEL.\n");
            fprintf(txt, "\tAVISO: O percurso '%s' é inacessível.\n", np);
            LinhaInacessivel* linha_dados = malloc(sizeof(LinhaInacessivel));
            linha_dados->no_inicio = p->no_inicio;
            linha_dados->no_fim = p->no_fim;

            ElementoVisual* el_visual = malloc(sizeof(ElementoVisual));
            el_visual->tipo = TIPO_LINHA_INACESSIVEL;
            el_visual->dados = linha_dados;
            lista_insere(elementos_visuais, el_visual);
            return;
        }
        printf("DEBUG: Percurso é ACESSÍVEL. Caminho curto tem %d nós. Caminho rápido tem %d nós.\n",
        lista_tamanho(p->caminho_curto), lista_tamanho(p->caminho_rapido));
               double base_duration = 8.0;
        double speed_factor = 1.5; 
        double tempo_curto = calcularCustoTotalPercurso(g, p->caminho_curto, CRITERIO_TEMPO, funcCusto);
        double tempo_rapido = calcularCustoTotalPercurso(g, p->caminho_rapido, CRITERIO_TEMPO, funcCusto);
        double duracao_curto;
        double duracao_rapido;

        if (tempo_rapido < tempo_curto) {
            duracao_rapido = base_duration / speed_factor; // Mais rápido
            duracao_curto = base_duration;                 // Mais lento
        } else {
            duracao_curto = base_duration / speed_factor;  // Mais rápido
            duracao_rapido = base_duration;                  // Mais lento
        }

        if (p->caminho_curto && !lista_vazia(p->caminho_curto)) {
            CaminhoVisual* cv_curto = malloc(sizeof(CaminhoVisual));
            cv_curto->caminho = p->caminho_curto;
            strcpy(cv_curto->cor, cmc);
            cv_curto->eh_curto = true; 
            cv_curto->duracao_animacao = duracao_curto;

            ElementoVisual* el_curto = malloc(sizeof(ElementoVisual));
            el_curto->tipo = TIPO_CAMINHO;
            el_curto->dados = cv_curto;
            lista_insere(elementos_visuais, el_curto);
        }

        if (p->caminho_rapido && !lista_vazia(p->caminho_rapido)) {
            CaminhoVisual* cv_rapido = malloc(sizeof(CaminhoVisual));
            cv_rapido->caminho = p->caminho_rapido;
            strcpy(cv_rapido->cor, cmr);
            cv_rapido->eh_curto = false; 
            cv_rapido->duracao_animacao = duracao_rapido;

            ElementoVisual* el_rapido = malloc(sizeof(ElementoVisual));
            el_rapido->tipo = TIPO_CAMINHO;
            el_rapido->dados = cv_rapido;
            lista_insere(elementos_visuais, el_rapido);
        }

    } else {
        printf("DEBUG: Percurso '%s' NÃO FOI ENCONTRADO na tabela hash.\n", np);
        fprintf(txt, "\tERRO: Percurso '%s' não encontrado.\n", np);
    }
}

static void executaAlag(const char* linha, Graph g, ResultadosConsultaImp* resultados, hashTable alagamentos, FILE* txt) {
    int n;
    double x, y, w, h;
    sscanf(linha, "alag %d %lf %lf %lf %lf", &n, &x, &y, &w, &h);
    fprintf(txt, "[alag*]\n\tÁrea de alagamento: n=%d, região (%.2f,%.2f) até (%.2f,%.2f)\n", n, x, y, x+w, y+h);
    
    Lista nos_na_regiao = lista_cria();

    bool encontrou_nos = getNodesInRegion(g, x, y, x + w, y + h, nos_na_regiao);
    
    if (!encontrou_nos || lista_vazia(nos_na_regiao)) {
        fprintf(txt, "\tAVISO: Nenhum nó encontrado na região especificada.\n");
        lista_libera(nos_na_regiao);
        return;
    }
    
    fprintf(txt, "\tNós encontrados na região: %d\n", lista_tamanho(nos_na_regiao));

    Lista arestas_afetadas = lista_cria();
    
    Iterador it_nos = lista_iterador(nos_na_regiao);
    while (iterador_tem_proximo(it_nos)) {
        NodeSmu no_da_arvore = (NodeSmu)iterador_proximo(it_nos);
        Node id_no_alagado = getNodeIdFromSmuTNode(g, no_da_arvore);
        
        if (id_no_alagado == -1) {
            fprintf(txt, "\tERRO: Não foi possível obter ID do nó da SmuTreap\n");
            continue;
        }
        
        fprintf(txt, "\tProcessando nó alagado: %s (ID: %d)\n", getNodeName(g, id_no_alagado), id_no_alagado);
        
        // Desabilita arestas de SAÍDA do nó alagado
        Lista arestas_saindo = lista_cria();
        adjacentEdges(g, id_no_alagado, arestas_saindo);
        Iterador it_saindo = lista_iterador(arestas_saindo);
        while (iterador_tem_proximo(it_saindo)) {
            Edge aresta = (Edge)iterador_proximo(it_saindo);
            if (isEdgeEnabled(g, aresta)) {
                disableEdge(g, aresta);
                lista_insere(arestas_afetadas, aresta);
                Node destino = getToNode(g, aresta);
                fprintf(txt, "\t  -> Aresta DESABILITADA: %s -> %s\n", 
                       getNodeName(g, id_no_alagado), getNodeName(g, destino));
            }
        }
        iterador_destroi(it_saindo);
        lista_libera(arestas_saindo);
        Lista arestas_chegando = lista_cria();
        incomingEdges(g, id_no_alagado, arestas_chegando);
        Iterador it_chegando = lista_iterador(arestas_chegando);
        while (iterador_tem_proximo(it_chegando)) {
            Edge aresta = (Edge)iterador_proximo(it_chegando);
            if (isEdgeEnabled(g, aresta)) {
                disableEdge(g, aresta);
                lista_insere(arestas_afetadas, aresta);
                Node origem = getFromNode(g, aresta);
                fprintf(txt, "\t  -> Aresta DESABILITADA: %s -> %s\n", 
                       getNodeName(g, origem), getNodeName(g, id_no_alagado));
            }
        }
        iterador_destroi(it_chegando);
        lista_libera(arestas_chegando);
    }
    iterador_destroi(it_nos);
    lista_libera(nos_na_regiao);
    
    fprintf(txt, "\tTotal de arestas desabilitadas: %d\n", lista_tamanho(arestas_afetadas));
    
    // Guardar informações do alagamento para o comando 'dren'
    InfoAlagamento* info_alag = malloc(sizeof(InfoAlagamento));
    info_alag->id = n;
    info_alag->arestas_desabilitadas = arestas_afetadas;
    lista_insere(resultados->vestiario_de_alagamentos, info_alag);
    int id_do_alagamento = lista_tamanho(resultados->vestiario_de_alagamentos) - 1;
    char chave_n[12];
    sprintf(chave_n, "%d", n);
    hashPut(alagamentos, chave_n, id_do_alagamento);
    RetanguloVisual* rv = malloc(sizeof(RetanguloVisual));
    rv->x = x; rv->y = y; rv->w = w; rv->h = h;
    strcpy(rv->cor_borda, "#AA0044");
    strcpy(rv->cor_fill, "#AB37C8");
    strcpy(rv->opacidade, "0.5");

    ElementoVisual* el = malloc(sizeof(ElementoVisual));
    el->tipo = TIPO_RETANGULO_ALAGAMENTO;
    el->dados = rv;
    lista_insere(resultados->elementos_para_desenhar, el);
}

static void executaDren(const char* linha, Graph g, ResultadosConsultaImp* resultados, hashTable alagamentos, FILE* txt) {
    int n;
    sscanf(linha, "dren %d", &n);
    fprintf(txt, "[dren*] Drenando região n=%d\n", n);

    char chave_n[12];
    sprintf(chave_n, "%d", n);
    
    int id_do_alagamento;
    if (hashGet(alagamentos, chave_n, &id_do_alagamento)) {
        InfoAlagamento* info_alag = (InfoAlagamento*)lista_get_por_indice(resultados->vestiario_de_alagamentos, id_do_alagamento);
    
        if (info_alag) {
            Iterador it = lista_iterador(info_alag->arestas_desabilitadas);
            int contagem = 0;
            
            fprintf(txt, "\tArestas reabilitadas:\n"); // Adiciona um cabeçalho para o relatório

            while (iterador_tem_proximo(it)) {
                Edge aresta = (Edge)iterador_proximo(it);
                Node origem = getFromNode(g, aresta);
                Node destino = getToNode(g, aresta);
                fprintf(txt, "\t  -> %s -> %s\n", getNodeName(g, origem), getNodeName(g, destino));
                contagem++;
            }
            iterador_destroi(it);
            fprintf(txt, "\t-> Total de %d arestas foram reabilitadas.\n", contagem);
        }
    } else {
        fprintf(txt, "\tERRO: Região de alagamento n=%d não encontrada.\n", n);
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
    resultados->elementos_para_desenhar = lista_cria();
    resultados->vestiario_de_percursos = calloc(MAX_PERCURSOS, sizeof(Percurso*));
    resultados->contador_de_fichas = 0;
    hashTable tabela_de_percursos = createHashTable(29);
    hashTable tabela_de_alagamentos = createHashTable(13);
    resultados->vestiario_de_alagamentos = lista_cria();
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
           executaAlag(buffer_linha, g, resultados, tabela_de_alagamentos, arquivo_txt);

        // COMANDO dren 
        } else if (strncmp(buffer_linha, "dren", 4) == 0) {
             executaDren(buffer_linha, g, resultados, tabela_de_alagamentos, arquivo_txt);
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
           executaPInterrogacao(buffer_linha, g, registradores, tabela_de_percursos, arquivo_txt, funcCusto, resultados->vestiario_de_percursos, &resultados->contador_de_fichas);
        } else if (strncmp(buffer_linha, "join", 4) == 0) {
            printf("processa join simulação");
            fprintf(arquivo_txt, "  -> Comando 'join' identificado, mas não implementado.\n");
            //1. Semelhante a p?

        // COMANDO shw 
        } else if (strncmp(buffer_linha, "shw", 3) == 0) {
            printf("\nDEBUG: Comando 'shw' RECONHECIDO. Chamando executaShw...\n");
            executaShw(buffer_linha, g, resultados->vestiario_de_percursos, tabela_de_percursos, resultados->elementos_para_desenhar, arquivo_txt, funcCusto);
        }

        fprintf(arquivo_txt, "\n");
    }
    printf("DEBUG: Fim do processamento. Total de elementos para desenhar: %d\n",
       lista_tamanho(resultados->elementos_para_desenhar));

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


    if (resultados->vestiario_de_percursos) {
        for (int i = 0; i < resultados->contador_de_fichas; i++) {
            if (resultados->vestiario_de_percursos[i]) {
                lista_libera(resultados->vestiario_de_percursos[i]->caminho_curto);
                lista_libera(resultados->vestiario_de_percursos[i]->caminho_rapido);
                free(resultados->vestiario_de_percursos[i]);
            }
        }
        free(resultados->vestiario_de_percursos);
    }

    free(resultados);
}


Lista getElementosParaDesenhar(ResultadosConsulta res) {
    if (!res) return NULL;
    ResultadosConsultaImp* resultados = (ResultadosConsultaImp*)res;
    return resultados->elementos_para_desenhar;
}