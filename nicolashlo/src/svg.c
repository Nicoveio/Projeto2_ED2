#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#include "svg.h"
#include "graph.h"
#include "lista.h"

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