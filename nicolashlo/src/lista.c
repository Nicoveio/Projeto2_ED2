#include "lista.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct no {
    Elemento dado;
    struct no* prox;
} No;

typedef struct iterador {
    No* atual;
} IteradorImp;

typedef struct lista {
    No* primeiro;
    int tamanho;
} ListaImp;

Lista lista_cria() {
    ListaImp* nova_lista = (ListaImp*)malloc(sizeof(ListaImp));
    if (!nova_lista) return NULL;

    nova_lista->primeiro = NULL;
    nova_lista->tamanho = 0;
    return (Lista)nova_lista;
}

void lista_insere(Lista lista, Elemento elemento) {
    if (!lista) return;

    /*printf("DEBUG (lista_insere): Inserindo elemento no endereço de memória %p\n", elemento);
    fflush(stdout);*/

    ListaImp* l = (ListaImp*)lista;

    No* novo_no = (No*)malloc(sizeof(No));
    if (!novo_no) return;

    novo_no->dado = elemento;
    novo_no->prox = l->primeiro;
    l->primeiro = novo_no;
    l->tamanho++;
}

Elemento lista_remove_primeiro(Lista lista) {
    if (!lista || lista_vazia(lista)) return NULL;
    ListaImp* l = (ListaImp*)lista;

    No* removido = l->primeiro;
    Elemento dado = removido->dado;

    l->primeiro = removido->prox;
    free(removido);
    l->tamanho--;

    return dado;
}

bool lista_vazia(Lista lista) {
    if (!lista) return true;
    ListaImp* l = (ListaImp*)lista;
    return l->primeiro == NULL;
}

void lista_libera(Lista lista) {
    if (!lista) return;
    ListaImp* l = (ListaImp*)lista;

    No* atual = l->primeiro;
    while (atual != NULL) {
        No* temp = atual;
        atual = atual->prox;
        free(temp);
    }
    free(l);
}

void lista_percorre(Lista lista, void (*func)(Elemento)) {
    if (!lista || !func) return;
    ListaImp* l = (ListaImp*)lista;

    for (No* atual = l->primeiro; atual != NULL; atual = atual->prox) {
        func(atual->dado);
    }
}

void lista_imprime(Lista lista, PrintElemento printaInfo) {
    lista_percorre(lista, printaInfo);
}
 int lista_tamanho(Lista l){
    if(!l)return 0;
    ListaImp* lista = (ListaImp*)l;
    return lista->tamanho;
 }
// ---------------------- Iterador Externo ----------------------

Iterador lista_iterador(Lista l) {
    if (!l) return NULL;
    ListaImp* lista = (ListaImp*)l;

    IteradorImp* it = (IteradorImp*)malloc(sizeof(IteradorImp));
    if (!it) return NULL;

    it->atual = lista->primeiro;
    return (Iterador)it;
}

bool iterador_tem_proximo(Iterador it) {
    if (!it) return false;

    IteradorImp* i = (IteradorImp*)it;
    return i->atual != NULL;
}

Elemento iterador_proximo(Iterador it) {
    if (!it) return NULL;

    IteradorImp* i = (IteradorImp*)it;
    if (!i->atual) return NULL;

    Elemento dado = i->atual->dado;
    i->atual = i->atual->prox;
    return dado;
}

void iterador_destroi(Iterador it) {
    free(it);
}

Elemento lista_get_por_indice(Lista lista, int indice) {
    if (!lista || indice < 0) return NULL;
    ListaImp* l = (ListaImp*)lista;
    if (indice >= l->tamanho) return NULL;

    No* atual = l->primeiro;
    for (int i = 0; i < indice; i++) {
        atual = atual->prox;
    }
    return atual->dado;
}

Elemento lista_get_primeiro(Lista lista) {
    if (!lista || lista_vazia(lista)) return NULL;
    ListaImp* l = (ListaImp*)lista;
    return l->primeiro->dado;
}

Elemento lista_get_ultimo(Lista lista) {
    if (!lista || lista_vazia(lista)) return NULL;
    ListaImp* l = (ListaImp*)lista;
    No* atual = l->primeiro;
    while (atual->prox != NULL) {
        atual = atual->prox;
    }
    return atual->dado;
}

Lista lista_concatena(Lista l1, Lista l2) {
    Lista nova_lista = lista_cria();
    ListaImp* nl = (ListaImp*)nova_lista;
    No* cauda = NULL; // Ponteiro para o último nó da nova lista

    // Copia os elementos de l1
    Iterador it1 = lista_iterador(l1);
    while (iterador_tem_proximo(it1)) {
        Elemento el = iterador_proximo(it1);
        // Insere no final da nova lista (lógica otimizada)
        No* novo_no = (No*)malloc(sizeof(No));
        novo_no->dado = el;
        novo_no->prox = NULL;
        if (cauda == NULL) {
            nl->primeiro = novo_no;
        } else {
            cauda->prox = novo_no;
        }
        cauda = novo_no;
        nl->tamanho++;
    }
    iterador_destroi(it1);

    Iterador it2 = lista_iterador(l2);
    while (iterador_tem_proximo(it2)) {
        Elemento el = iterador_proximo(it2);
        No* novo_no = (No*)malloc(sizeof(No));
        novo_no->dado = el;
        novo_no->prox = NULL;
        if (cauda == NULL) {
            nl->primeiro = novo_no;
        } else {
            cauda->prox = novo_no;
        }
        cauda = novo_no;
        nl->tamanho++;
    }
    iterador_destroi(it2);

    return nova_lista;
}
