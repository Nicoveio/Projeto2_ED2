#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>

typedef void* Elemento;
typedef void* Lista;
typedef void* Iterador;

typedef void (*PrintElemento)(Elemento);

Lista lista_cria();
void lista_insere(Lista lista, Elemento elemento);
Elemento lista_remove_primeiro(Lista lista);
bool lista_vazia(Lista lista);
void lista_libera(Lista lista);

void lista_percorre(Lista lista, void (*func)(Elemento));
void lista_imprime(Lista lista, PrintElemento printaInfo);
int lista_tamanho(Lista l);

// Iterador externo
Iterador lista_iterador(Lista l);
bool iterador_tem_proximo(Iterador it);
Elemento iterador_proximo(Iterador it);
void iterador_destroi(Iterador it);
Elemento lista_get_por_indice(Lista lista, int indice);
#endif
