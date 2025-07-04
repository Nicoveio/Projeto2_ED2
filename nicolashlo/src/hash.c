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
// procurei por um algoritmo de hash de strings eficaz, e encontrei o djb2 do Daniel J. Bernstein, matemático americano
static unsigned long int hash_djb2(char *chave){
	unsigned long hash = 5381;
	int x;
	while((x=*chave++)){
		hash = ((hash<<5)+ hash) + x;
	}
	return hash;
}

static int balde_index(char*chave, int size){
	return hash_djb2(chave) % size;
}