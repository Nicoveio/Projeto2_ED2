#ifndef FILA_H
#define FILA_H

#include <stdbool.h>

typedef void* Elemento;  // Tipo genérico para elementos da fila (ponteiro para qualquer dado)
typedef void* Fila;      // Tipo opaco para fila, implementação oculta

/**
 * Cria uma fila vazia com capacidade inicial.
 * @param capacidade_inicial Capacidade inicial da fila.
 * @return Ponteiro para a fila criada, ou NULL em caso de falha.
 */
Fila fila_cria();

/**
 * Libera toda a memória ocupada pela fila.
 * @param f Ponteiro para a fila a ser liberada.
 */
void fila_libera(Fila f);

/**
 * Verifica se a fila está vazia.
 * @param f Ponteiro para a fila.
 * @return true se a fila estiver vazia; false caso contrário.
 */
bool fila_vazia(Fila f);

/**
 * Insere um elemento no final da fila.
 * @param f Ponteiro para a fila.
 * @param elem Elemento a ser inserido (ponteiro genérico).
 * @return true se a inserção foi bem-sucedida; false caso contrário.
 */
bool fila_insere(Fila f, Elemento elem);

/**
 * Remove e retorna o elemento do início da fila.
 * @param f Ponteiro para a fila.
 * @return Elemento removido, ou NULL se a fila estiver vazia.
 */
Elemento fila_remove(Fila f);

#endif
