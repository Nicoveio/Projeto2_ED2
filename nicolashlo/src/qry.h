#ifndef QRY_H
#define QRY_H

#include "graph.h"
#include "lista.h"

/**
 * Define um tipo opaco para armazenar os resultados visuais das consultas
 * (caminhos a serem desenhados, retângulos de alagamento, etc.).
 */
typedef void* ResultadosConsulta;

/**
 * Processa um arquivo de consultas (.qry), executando cada comando.
 * @param g O grafo da cidade, já carregado.
 * @param quadras A lista de quadras da cidade.
 * @param caminho_qry O caminho para o arquivo .qry.
 * @param caminho_txt_saida O caminho para o arquivo .txt de saída a ser criado.
 * @return Um ponteiro para uma estrutura ResultadosConsulta, contendo os
 * elementos visuais a serem desenhados no SVG. O chamador é
 * responsável por liberar esta estrutura.
 */
ResultadosConsulta processaQry(Graph g, Lista quadras, const char* caminho_qry, const char* caminho_txt_saida);

/**
 * Obtém a lista de elementos visuais a serem desenhados a partir de
 * uma estrutura de resultados de consulta.
 * @param resultados O ponteiro opaco para a estrutura de resultados.
 * @return A Lista de elementos visuais.
 */
Lista getElementosParaDesenhar(ResultadosConsulta resultados);

/**
 * Libera a memória da estrutura de resultados da consulta.
 * @param resultados O ponteiro para a estrutura a ser liberada.
 */
void liberaResultadosConsulta(ResultadosConsulta resultados);

#endif /* QRY_H */