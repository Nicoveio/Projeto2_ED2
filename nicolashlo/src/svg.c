#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdint.h>

#include "svg.h"
#include "graph.h"
#include "lista.h"
#include "qry.h"

#define MARGEM_VIEWBOX 50.0

// As structs que o SVG precisa conhecer para desenhar
typedef struct {
    char cep[50];
    double x, y, w, h;
    char cfill[25];
    char cstrk[25];
    char sw[8];
} Quadra;

typedef struct {
    double x;
    double y;
} Coordenadas;


typedef enum {
    TIPO_MARCADOR_O,
    TIPO_CAMINHO,
    TIPO_LINHA_INACESSIVEL,
    TIPO_RETANGULO_ALAGAMENTO

} TipoElementoVisual;

typedef struct {
    char registrador[8]; 
    double x; 
    double y;
    char face;
} MarcadorO;

typedef struct { 
    TipoElementoVisual tipo; 
    void* dados; } ElementoVisual;

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


// ====================================================================
// FUNÇÕES DE DESENHO (INTERNAS)
// ====================================================================

static void desenharQuadras(FILE* arq, Lista quadras) {
    if (lista_vazia(quadras)) return;

    fprintf(arq, "\n  \n");
    Iterador it = lista_iterador(quadras);
    while (iterador_tem_proximo(it)) {
        Quadra* q = (Quadra*)iterador_proximo(it);
        fprintf(arq, "  <svg:rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" stroke=\"black\" fill=\"%s\" stroke-width=\"1\" />\n",
                q->x, q->y, q->w, q->h, q->cfill);
        fprintf(arq, "  <svg:text x=\"%.2f\" y=\"%.2f\" fill=\"black\" font-weight=\"bold\" font-size=\"12\" text-anchor=\"middle\" dominant-baseline=\"middle\">%s</svg:text>\n",
                q->x + q->w / 2, q->y + q->h / 2, q->cep);
    }
    iterador_destroi(it);
}

static void desenharRuasEVertices(FILE* arq, Graph g) {
    if (getTotalNodes(g) == 0) return;

    // --- 1. Desenha as RUAS (ARCOS) ---
    fprintf(arq, "\n  \n");
    for (Node i = 0; i < getTotalNodes(g); i++) {
        Lista arestas = lista_cria();
        adjacentEdges(g, i, arestas);
        if (lista_vazia(arestas)) {
            lista_libera(arestas);
            continue;
        }

        Coordenadas* coord_origem = (Coordenadas*)getNodeInfo(g, i);
        Iterador it = lista_iterador(arestas);
        while (iterador_tem_proximo(it)) {
            Edge e = (Edge)iterador_proximo(it);
            Node id_destino = getToNode(g, e);
            Coordenadas* coord_destino = (Coordenadas*)getNodeInfo(g, id_destino);
            
            // Desenha a linha e adiciona o 'marker-end' para a seta aparecer
            fprintf(arq, "  <svg:line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"black\" stroke-width=\"1\" marker-end=\"url(#mArrow)\" />\n",
                    coord_origem->x, coord_origem->y, coord_destino->x, coord_destino->y);
        }
        iterador_destroi(it);
        lista_libera(arestas);
    }
    
    // --- 2. Desenha os VÉRTICES e NOMES por cima ---
    fprintf(arq, "\n  \n");
    for (Node i = 0; i < getTotalNodes(g); i++) {
        Coordenadas* coord = (Coordenadas*)getNodeInfo(g, i);
        char* nome_vertice = getNodeName(g, i);

        // Desenha o círculo do vértice com o estilo do gabarito
        fprintf(arq, "  <svg:circle cx=\"%.2f\" cy=\"%.2f\" r=\"4\" fill=\"blue\" stroke=\"black\" fill-opacity=\"0.5\" />\n", coord->x, coord->y);
        
        // Escreve o nome do vértice um pouco acima
        fprintf(arq, "  <svg:text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" font-size=\"8\" dy=\"-5\">%s</svg:text>\n",
                coord->x, coord->y, nome_vertice);
    }
}

// ====================================================================
// FUNÇÃO PÚBLICA PRINCIPAL
// ====================================================================

void gerarSvgBase(Graph g, Lista quadras, const char* caminho_svg) {
    if (!caminho_svg) return;

    FILE* arquivo = fopen(caminho_svg, "w");
    if (!arquivo) {
        fprintf(stderr, "ERRO: Não foi possível criar o arquivo SVG em '%s'\n", caminho_svg);
        return;
    }

    // --- 1. Calcular as dimensões do conteúdo ---
    double min_x = DBL_MAX, max_x = DBL_MIN, min_y = DBL_MAX, max_y = DBL_MIN;
    // ... (percorrendo a lista de quadras e os vértices do grafo para encontrar os limites) ...
    if (quadras && !lista_vazia(quadras)) {
        Iterador it_q = lista_iterador(quadras);
        while (iterador_tem_proximo(it_q)) {
            Quadra* q = (Quadra*)iterador_proximo(it_q);
            if (q->x < min_x) min_x = q->x;
            if (q->y < min_y) min_y = q->y;
            if (q->x + q->w > max_x) max_x = q->x + q->w;
            if (q->y + q->h > max_y) max_y = q->y + q->h;
        }
        iterador_destroi(it_q);
    }
    if (g && getTotalNodes(g) > 0) {
        for (Node i = 0; i < getTotalNodes(g); i++) {
            Coordenadas* c = (Coordenadas*)getNodeInfo(g, i);
            if (c->x < min_x) min_x = c->x;
            if (c->y < min_y) min_y = c->y;
            if (c->x > max_x) max_x = c->x;
            if (c->y > max_y) max_y = c->y;
        }
    }
    
    // Calcula os parâmetros do viewBox
    double vb_x = min_x - MARGEM_VIEWBOX;
    double vb_y = min_y - MARGEM_VIEWBOX;
    double vb_w = (max_x - min_x) + (2 * MARGEM_VIEWBOX);
    double vb_h = (max_y - min_y) + (2 * MARGEM_VIEWBOX);

    // --- 2. Escrever o Cabeçalho do SVG (usando o estilo do professor) ---
    fprintf(arquivo, "<?xml version='1.0' encoding='utf-8'?>\n");
    fprintf(arquivo, "<svg:svg xmlns:svg=\"http://www.w3.org/2000/svg\" viewBox=\"%.2f %.2f %.2f %.2f\">\n", vb_x, vb_y, vb_w, vb_h);
    fprintf(arquivo, "<svg:defs>\n");
    fprintf(arquivo, "  <svg:marker id=\"mArrow\" markerWidth=\"8\" markerHeight=\"8\" refX=\"0\" refY=\"4.0\" orient=\"auto\">\n");
    fprintf(arquivo, "    <svg:path d=\"M0,0 L0,8 L8,4 z\" style=\"fill: #000000;\" />\n");
    fprintf(arquivo, "  </svg:marker>\n");
    fprintf(arquivo, "</svg:defs>\n");
    fprintf(arquivo, "  <rect width=\"100%%\" height=\"100%%\" fill=\"#ced4da\" />\n");

    // --- 3. Desenhar o conteúdo ---
    fprintf(arquivo, "<svg:g id=\"mapa_base\">\n");

    desenharQuadras(arquivo, quadras);
    desenharRuasEVertices(arquivo, g);

    fprintf(arquivo, "</svg:g>\n");

    // --- 4. Escrever o Rodapé ---
    fprintf(arquivo, "</svg:svg>\n");

    fclose(arquivo);
    printf("SVG base gerado com sucesso: %s\n", caminho_svg);
}


static void desenharMarcadorO(FILE* arq, MarcadorO* marcador) {
    // Verifica se a face é horizontal (Norte ou Sul)
    if (marcador->face == 'N' || marcador->face == 'n' || marcador->face == 'S' || marcador->face == 's') {
        // Desenha uma LINHA VERTICAL do topo (y=0) até o ponto
        fprintf(arq, "  <svg:line x1=\"%.2f\" y1=\"0\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"red\" stroke-width=\"1\" stroke-dasharray=\"5,5\" />\n",
                marcador->x, marcador->x, marcador->y);
        // Escreve o texto no topo da linha
        fprintf(arq, "  <svg:text x=\"%.2f\" y=\"0\" fill=\"red\" font-size=\"10\" dy=\"-5\">%s</svg:text>\n",
                marcador->x, marcador->registrador);
    } else { // A face é vertical (Leste ou Oeste)
        // Desenha uma LINHA HORIZONTAL da esquerda (x=0) até o ponto
        fprintf(arq, "  <svg:line x1=\"0\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"red\" stroke-width=\"1\" stroke-dasharray=\"5,5\" />\n",
                marcador->y, marcador->x, marcador->y);
        // Escreve o texto na ponta esquerda da linha
        fprintf(arq, "  <svg:text x=\"0\" y=\"%.2f\" fill=\"red\" font-size=\"10\" dx=\"5\">%s</svg:text>\n",
                marcador->y, marcador->registrador);
    }
}
static void desenharCaminho(FILE* arq, Graph g, CaminhoVisual* caminho_visual, int* path_id_counter) {
    if (lista_vazia(caminho_visual->caminho)) return;

    char path_string[16384] = "M ";
    char temp[100];

    // Define o deslocamento. O caminho curto fica um pouco à "esquerda/cima", o rápido um pouco à "direita/baixo".
    double offset = caminho_visual->eh_curto ? -3.0 : 3.0;

    Iterador it = lista_iterador(caminho_visual->caminho);
    while(iterador_tem_proximo(it)) {
        Node no_id = (Node)(uintptr_t)iterador_proximo(it);
        Coordenadas* c = (Coordenadas*)getNodeInfo(g, no_id);
        // Aplica o offset às coordenadas antes de adicioná-las à string
        sprintf(temp, "%.2f,%.2f L ", c->x + offset, c->y + offset);
        strcat(path_string, temp);
    }
    iterador_destroi(it);
    path_string[strlen(path_string) - 2] = '\0';

    char path_id[20];
    sprintf(path_id, "path_%d", (*path_id_counter)++);
    fprintf(arq, "  <svg:path id=\"%s\" d=\"%s\" stroke=\"%s\" stroke-width=\"8.0\" fill=\"none\" stroke-opacity=\"2.0\" />\n", 
            path_id, path_string, caminho_visual->cor);

    fprintf(arq, "  <svg:image href=\"https://www.uel.br/pessoal/bacarin/figs/eb.jpg\" width=\"40\" height=\"40\" transform=\"translate(-20,-20)\">\n");
    fprintf(arq, "    <svg:animateMotion dur=\"%.2fs\" repeatCount=\"indefinite\">\n", caminho_visual->duracao_animacao);
    fprintf(arq, "      <svg:mpath href=\"#%s\"/>\n", path_id);
    fprintf(arq, "    </svg:animateMotion>\n");
    fprintf(arq, "  </svg:image>\n");


}
static void desenharLinhaInacessivel(FILE* arq, Graph g, LinhaInacessivel* linha) {
    if (!linha) return;

    // Pega as coordenadas dos nós de início e fim
    Coordenadas* coord_inicio = (Coordenadas*)getNodeInfo(g, linha->no_inicio);
    Coordenadas* coord_fim = (Coordenadas*)getNodeInfo(g, linha->no_fim);

    if (coord_inicio && coord_fim) {
        // Desenha a linha pontilhada vermelha
        fprintf(arq, "  <svg:line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" stroke=\"red\" stroke-width=\"2\" stroke-dasharray=\"5,5\" />\n",
                coord_inicio->x, coord_inicio->y,
                coord_fim->x, coord_fim->y);
    }
}

void gerarSvgFinal(Graph g, Lista quadras, ResultadosConsulta res, const char* caminho_svg) {
    if (!caminho_svg) return;

    FILE* arquivo = fopen(caminho_svg, "w");
    if (!arquivo) {
        fprintf(stderr, "ERRO: Não foi possível criar o arquivo SVG em '%s'\n", caminho_svg);
        return;
    }

    // --- 1. Calcular as dimensões do conteúdo ---
    double min_x = DBL_MAX, max_x = DBL_MIN, min_y = DBL_MAX, max_y = DBL_MIN;
    // ... (percorrendo a lista de quadras e os vértices do grafo para encontrar os limites) ...
    if (quadras && !lista_vazia(quadras)) {
        Iterador it_q = lista_iterador(quadras);
        while (iterador_tem_proximo(it_q)) {
            Quadra* q = (Quadra*)iterador_proximo(it_q);
            if (q->x < min_x) min_x = q->x;
            if (q->y < min_y) min_y = q->y;
            if (q->x + q->w > max_x) max_x = q->x + q->w;
            if (q->y + q->h > max_y) max_y = q->y + q->h;
        }
        iterador_destroi(it_q);
    }
    if (g && getTotalNodes(g) > 0) {
        for (Node i = 0; i < getTotalNodes(g); i++) {
            Coordenadas* c = (Coordenadas*)getNodeInfo(g, i);
            if (c->x < min_x) min_x = c->x;
            if (c->y < min_y) min_y = c->y;
            if (c->x > max_x) max_x = c->x;
            if (c->y > max_y) max_y = c->y;
        }
    }
    
    // Calcula os parâmetros do viewBox
    double vb_x = min_x - MARGEM_VIEWBOX;
    double vb_y = min_y - MARGEM_VIEWBOX;
    double vb_w = (max_x - min_x) + (2 * MARGEM_VIEWBOX);
    double vb_h = (max_y - min_y) + (2 * MARGEM_VIEWBOX);

    // --- 2. Escrever o Cabeçalho do SVG (usando o estilo do professor) ---
    fprintf(arquivo, "<?xml version='1.0' encoding='utf-8'?>\n");
    fprintf(arquivo, "<svg:svg xmlns:svg=\"http://www.w3.org/2000/svg\" viewBox=\"%.2f %.2f %.2f %.2f\">\n", vb_x, vb_y, vb_w, vb_h);
    fprintf(arquivo, "<svg:defs>\n");
    fprintf(arquivo, "  <svg:marker id=\"mArrow\" markerWidth=\"8\" markerHeight=\"8\" refX=\"0\" refY=\"4.0\" orient=\"auto\">\n");
    fprintf(arquivo, "    <svg:path d=\"M0,0 L0,8 L8,4 z\" style=\"fill: #000000;\" />\n");
    fprintf(arquivo, "  </svg:marker>\n");
    fprintf(arquivo, "</svg:defs>\n");
    fprintf(arquivo, "  <rect width=\"100%%\" height=\"100%%\" fill=\"#ced4da\" />\n");

// --- DESENHAR O MAPA BASE ---
    fprintf(arquivo, "<svg:g id=\"mapa_base\">\n");
    desenharQuadras(arquivo, quadras);
    desenharRuasEVertices(arquivo, g);
    fprintf(arquivo, "</svg:g>\n");
    fprintf(arquivo, "<svg:g id=\"animacoes\">\n");

    fprintf(arquivo, "\n  \n");
    Lista elementos = getElementosParaDesenhar(res); // <-- A MUDANÇA
    printf("DEBUG: A função gerarSvgFinal recebeu uma lista com %d elementos para desenhar.\n",lista_tamanho(elementos));
    if (elementos) {
        Iterador it = lista_iterador(elementos);
        int path_id_counter = 0;
        while(iterador_tem_proximo(it)){
            ElementoVisual* el = (ElementoVisual*)iterador_proximo(it);
            switch (el->tipo) {
                case TIPO_MARCADOR_O:{
                desenharMarcadorO(arquivo, (MarcadorO*)el->dados);
                break;
                }

                case TIPO_CAMINHO:{
                fprintf(arquivo, "<svg:g>\n"); 
                desenharCaminho(arquivo, g, (CaminhoVisual*)el->dados, &path_id_counter);
                fprintf(arquivo, "</svg:g>\n");
                break;
                }
                case TIPO_LINHA_INACESSIVEL:{
                desenharLinhaInacessivel(arquivo, g, (LinhaInacessivel*)el->dados);
                break;
                }


            }

        }

}
    fprintf(arquivo, "</svg:g>\n");
    // --- 4. Escrever o Rodapé ---
    fprintf(arquivo, "</svg:svg>\n");

    fclose(arquivo);
    printf("SVG base gerado com sucesso: %s\n", caminho_svg);


}