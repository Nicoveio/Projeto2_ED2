#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "fila.h" // A interface não muda

// --- NOVAS ESTRUTURAS INTERNAS ---

// Nó da lista encadeada. Cada nó guarda um elemento e aponta para o próximo.
typedef struct no_fila {
    Elemento dado;
    struct no_fila* prox;
} NoFila;

// A estrutura da fila agora guarda ponteiros para o primeiro e o último nó.
typedef struct fila_imp {
    NoFila* inicio;
    NoFila* fim;
    int tamanho;
} FilaImp;


// --- IMPLEMENTAÇÃO DAS FUNÇÕES PÚBLICAS ---

Fila fila_cria() {
    FilaImp* f = (FilaImp*) malloc(sizeof(FilaImp));
    if (!f) return NULL;

    f->inicio = NULL;
    f->fim = NULL;
    f->tamanho = 0;
    return (Fila) f;
}

void fila_libera(Fila fila) {
    if (!fila) return;

    // Remove todos os elementos para liberar a memória de cada nó
    while (!fila_vazia(fila)) {
        fila_remove(fila);
    }
    // Libera a estrutura da fila em si
    free(fila);
}

bool fila_insere(Fila fila, Elemento elem) {
    if (!fila) return false;

    FilaImp* f = (FilaImp*) fila;

    // 1. Cria o novo nó que entrará na fila
    NoFila* novo_no = (NoFila*) malloc(sizeof(NoFila));
    if (!novo_no) return false;
    novo_no->dado = elem;
    novo_no->prox = NULL; // O novo nó sempre será o último, então não aponta para ninguém

    // 2. Adiciona o nó no final da fila
    if (fila_vazia(fila)) {
        // Se a fila está vazia, o novo nó é tanto o início quanto o fim
        f->inicio = novo_no;
        f->fim = novo_no;
    } else {
        // Se não, o 'prox' do antigo último nó aponta para o novo
        f->fim->prox = novo_no;
        // E o novo nó se torna o último
        f->fim = novo_no;
    }

    f->tamanho++;
    return true;
}

Elemento fila_remove(Fila fila) {
    if (!fila || fila_vazia(fila)) return NULL;

    FilaImp* f = (FilaImp*) fila;

    // 1. Guarda o nó do início para podermos liberar sua memória
    NoFila* no_removido = f->inicio;
    Elemento dado = no_removido->dado;

    // 2. Avança o início da fila para o próximo elemento
    f->inicio = f->inicio->prox;

    // 3. Caso especial: se a fila ficou vazia após a remoção,
    //    o fim também deve ser NULL.
    if (f->inicio == NULL) {
        f->fim = NULL;
    }

    // 4. Libera a memória do nó que foi removido
    free(no_removido);
    f->tamanho--;

    return dado;
}

bool fila_vazia(Fila fila) {
    if (!fila) return true;
    FilaImp* f = (FilaImp*) fila;
    return f->tamanho == 0;
}