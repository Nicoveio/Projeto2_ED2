// Arquivo: via.h
#ifndef VIA_H
#define VIA_H

#include "graph.h" // Precisa saber o que é o tipo 'Graph'

/**
 * @brief Calcula o custo (peso) de uma aresta com base em um critério específico.
 * * Esta função atua como uma "calculadora" para o algoritmo A*. Ela recebe a
 * informação genérica de uma aresta e, sabendo que se trata de uma InfoAresta
 * deste projeto, extrai o dado relevante (comprimento ou tempo) conforme o
 * critério solicitado.
 * * @param info_aresta Um ponteiro genérico (Info) para a struct InfoAresta.
 * @param criterio    Um inteiro que especifica qual métrica de custo deve ser
 * utilizada (CRITERIO_DISTANCIA ou CRITERIO_TEMPO).
 * @return O custo (peso) calculado da aresta, como um valor double.
 */
double calculaCustoAresta(Info info_aresta, int criterio);
/**
 * Lê um arquivo de vias (.via), que descreve a malha viária de uma cidade.
 * A função cria e popula um objeto Graph com todos os vértices (cruzamentos)
 * e arestas (segmentos de rua) descritos no arquivo.
 * @param caminho_via O caminho completo para o arquivo .via a ser lido.
 * @return Um ponteiro para o objeto Graph preenchido, ou NULL em caso de erro
 * (ex: arquivo não encontrado).
 */
Graph carregarGrafoDeArquivoVia(const char* caminho_via);

#endif /* VIA_H */