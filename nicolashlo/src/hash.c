#include "hash.h"
#include <stdlib.h>

typedef struct Node{
	char* chave;
	int valor;
	struct Node* prox;
}Node;

typedef struct{
	Node** balde;
	int size;
}tabelaHash;

hashTable createHashTable(int size){
	if(size<=0)return NULL;
	tabelaHash* ht = (tabelaHash*)malloc(sizeof(tabelaHash));
	if(!ht){
		printf("Erro de alocação de memória da tabela.\n");
		exit(1);
	}
	ht->balde = (Node**)malloc(size*sizeof(Node*));
	if(!ht->balde){
		printf("Erro de alocação de memória do balde.\n");
		exit(1);
	}
	for(int i=0; i<size; i++){
		ht->balde[i] = NULL;
	}
	ht->size = size;
	return (void*)ht;
}