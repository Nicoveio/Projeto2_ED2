#ifndef SVG_H
#define SVG_H
#include "graph.h"
#include "lista.h"
#include "qry.h"
/**
 * Gera o arquivo SVG base, desenhando o mapa estático da cidade.
 * Inclui todas as quadras e a malha viária completa.
 * @param g O grafo da cidade, contendo os vértices e arestas.
 * @param quadras Uma lista contendo ponteiros para as structs Quadra.
 * @param caminho_svg O nome do arquivo SVG a ser criado.
 */
void gerarSvgBase(Graph g, Lista quadras, const char* caminho_svg);

void gerarSvgFinal(Graph g, Lista quadras, ResultadosConsulta res, const char* caminho_svg);

#endif