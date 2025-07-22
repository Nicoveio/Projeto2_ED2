// Arquivo: via.h
#ifndef VIA_H
#define VIA_H

#include "graph.h" // Precisa saber o que é o tipo 'Graph'

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