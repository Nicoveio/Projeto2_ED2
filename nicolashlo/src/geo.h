#ifndef GEO_H
#define GEO_H
#include "lista.h" // Porque a função vai retornar uma Lista

/**
 * Lê um arquivo .geo, processa os comandos de quadra (q) e
 * cor (cq), e retorna uma lista com todas as quadras da cidade.
 * @param caminho_geo O caminho completo para o arquivo .geo.
 * @return Uma Lista contendo ponteiros para structs Quadra,
 * ou NULL em caso de erro.
 */
Lista processaGeo(const char* caminho_geo);

#endif