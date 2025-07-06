#include "hash.h"
#include <stdlib.h>
#include <string.h>

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
static unsigned long int hash_djb2(const char *chave){
	unsigned long hash = 5381;
	int x;
	while((x=*chave++)){
		hash = ((hash<<5)+ hash) + x;
	}
	return hash;
}

static int balde_index(const char*chave, int size){
	return hash_djb2(chave) % size;
}

void hashPut(hashTable ht, const char * key, int value){
	tabelaHash* ht0 = (tabelaHash*)ht;
	int indice = balde_index(key, ht0->size);
	Node* atual = ht0->balde[indice];
	while(atual){
		if(strcmp(key, atual->chave)==0){
			atual->valor = value;
			return;
		}
		atual=atual->prox;
	}
	Node* novo = (Node*)malloc(sizeof(Node));
	if(!novo){
		printf("Problema de alocação de memoria de nó na hashTable.");
		exit(1);
	}
	novo->chave = strdup(key);
	novo->valor = value;
	novo->prox = ht0->balde[indice];
	ht0->balde[indice] = novo;

}

bool hashGet(hashTable ht, const char*key, int* value){
	if(!ht || !key || !value ) return false;
	tabelaHash* ht0 = (tabelaHash*)ht;
	int indice = balde_index(key, ht0->size);
	Node* atual = ht0->balde[indice];
	while(atual){
		if(strcmp(key, atual->chave)==0){
			*value = atual->valor;
			return true;
		}
		else atual = atual->prox;
	}
	return false;


}

void hashRemove(hashTable ht, const char* key){
	if(!ht || !key)return;
	tabelaHash* ht0 = (tabelaHash*)ht;
	int indice = balde_index(key, ht0->size);
	Node* atual = ht0->balde[indice];
	Node* anterior = NULL;
	while(atual){
		if(strcmp(key, atual->chave)==0){
			if(!anterior)
				ht0->balde[indice] = atual ->prox;
			else anterior->prox= atual->prox;
			free(atual->chave);
			free(atual);
			return;
		}else{
			anterior = atual;
			atual = atual->prox;
		}
	}
}


void hashTableDestroy(hashTable ht){
	if(!ht) return;
	tabelaHash* ht0 = (tabelaHash*)ht;
	for(int i=0; i<ht0->size; i++){
		Node* atual = ht0->balde[i];
		while(atual){
			Node* libera = atual;
			atual = atual->prox;
			free(libera->chave);
			free(libera);
		}
	}
	free(ht0->balde);
	free(ht0);

}

