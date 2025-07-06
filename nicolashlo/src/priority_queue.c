#include "priority_queue.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
	int item;
	double prioridade;
}HeapNode;

typedef struct{
	HeapNode *heap;
	int tamanhoAtual;
	int capacidadeTotal;
}FilaPrioridade;

static void sobeNoAux(FilaPrioridade* fp, int ultimoIndice);
static void desceNoAux(FilaPrioridade* fp, int indice);

priorityQueue createPriorityQueue(int capacidade){
	if(capacidade<=0)return NULL;

	FilaPrioridade* fp = (FilaPrioridade*)malloc(sizeof(FilaPrioridade));
	if(!fp){
		printf("erro de alocação de memoria pra fila.\n");
		exit(1);
	}
	capacidade++;
	fp->heap = (HeapNode*)malloc(capacidade*sizeof(HeapNode));
	if(!fp->heap){
		printf("erro de alocação de memoria pra fila.\n");
		free(fp);
		exit(1);
	}
	fp->capacidadeTotal = capacidade-1;
	fp->tamanhoAtual = 0;
	return (void*)fp;
}

void pq_insert(priorityQueue pq, int item, double priority){
	FilaPrioridade* fp = (FilaPrioridade*)pq;
	if(fp->tamanhoAtual >= fp->capacidadeTotal){
		printf("fila de prioridade cheia.");
		return;
	}
	fp->tamanhoAtual++;
	fp->heap[fp->tamanhoAtual].item = item;
	fp->heap[fp->tamanhoAtual].prioridade = priority;
	sobeNoAux(fp, fp->tamanhoAtual);

}

static void sobeNoAux(FilaPrioridade* fp, int ultimoIndice){
	if(ultimoIndice<=1)
		return;

	int indicePai = ultimoIndice/2;
	if(fp->heap[ultimoIndice].prioridade<fp->heap[indicePai].prioridade){
		HeapNode temp = fp->heap[indicePai];
		fp->heap[indicePai]=fp->heap[ultimoIndice];
		fp->heap[ultimoIndice] =  temp;
		sobeNoAux(fp, indicePai);
	}
}

int pq_extract_min (priorityQueue pq){
	if(pq_empty(pq)) return -1;
	FilaPrioridade* fp = (FilaPrioridade*)pq;

	int itemReturn = fp->heap[1].item;
	fp->heap[1]=fp->heap[fp->tamanhoAtual];
	fp->tamanhoAtual--;
	desceNoAux(fp, 1);
	return itemReturn;
}
static void desceNoAux(FilaPrioridade* fp, int indice){
	int indiceMenor = indice;
	int indiceEsq = indice *2;
	int indiceDir = 2 * indice + 1;
	if(indiceEsq<=fp->tamanhoAtual && fp->heap[indiceEsq].prioridade<fp->heap[indiceMenor].prioridade)
		indiceMenor = indiceEsq;
	if (indiceDir<=fp->tamanhoAtual && fp->heap[indiceDir].prioridade<fp->heap[indiceMenor].prioridade) 
		indiceMenor = indiceDir;
	if(indiceMenor != indice){
		HeapNode temp = fp->heap[indice];
		fp->heap[indice]=fp->heap[indiceMenor];
		fp->heap[indiceMenor] =  temp;
		desceNoAux(fp, indiceMenor);
	}

}
bool pq_empty(priorityQueue pq) {
    if (!pq) {
        return true;
    }
    FilaPrioridade* fp = (FilaPrioridade*)pq;
    return fp->tamanhoAtual == 0;
}

void pq_destroy(priorityQueue pq) {
    if (!pq) {
        return;
    }
    FilaPrioridade* fp = (FilaPrioridade*)pq;
    free(fp->heap);
    free(fp);
}

