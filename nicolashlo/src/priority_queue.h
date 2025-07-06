#ifndef _PRIORITY_QUEUE
#define _PRIORITY_QUEUE

#include <stdbool.h>

/*Define o tipo opaco referente a fila de prioridade.*/

typedef void*priorityQueue;

/* 
Cria e inicializa a fila com uma capacidade máxima.
*/

priorityQueue createPriorityQueue(int capacidade);

/*
 Insere um item na fila com sua respectiva prioridade.
 Quanto menor o valor 'priority', maior sua prioridade.
*/

void pq_insert(priorityQueue pq, int item, double priority);

/* 
 Remove e retorna o item de maior prioridade 
 (menor valor de 'priority' da fila, o 'extraindo'.
*/

int pq_extract_ min(priorityQueue pq);

/* 
 Verica se a fila de prioridade 'pq' está vazia/não contém elementos. 
*/

bool pq_empty(priorityQueue pq);
/*
Libera toda memória da fila de prioridade 'pq'.
*/
void pq_destroy(priorityQueue pq);

#endif 