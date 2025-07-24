#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct HashNode{
	char* chave;
	int valor;
	struct HashNode* prox;
}HashNode;

typedef struct{
	HashNode** balde;
	int size;
}tabelaHash;

hashTable createHashTable(int size){
	if(size<=0)return NULL;
	tabelaHash* ht = (tabelaHash*)malloc(sizeof(tabelaHash));
	if(!ht){
		printf("Erro de alocação de memória da tabela.\n");
		exit(1);
	}
	ht->balde = (HashNode**)malloc(size*sizeof(HashNode*));
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
	HashNode* atual = ht0->balde[indice];
	while(atual){
		if(strcmp(key, atual->chave)==0){
			atual->valor = value;
			return;
		}
		atual=atual->prox;
	}
	HashNode* novo = (HashNode*)malloc(sizeof(HashNode));
	if(!novo){
		printf("Problema de alocação de memoria de nó na hashTable.");
		exit(1);
	}
	novo->chave = duplicar_string(key);
	novo->valor = value;
	novo->prox = ht0->balde[indice];
	ht0->balde[indice] = novo;

}

bool hashGet(hashTable ht, const char*key, int* value){
	if(!ht || !key || !value ) return false;
	tabelaHash* ht0 = (tabelaHash*)ht;
	int indice = balde_index(key, ht0->size);
	HashNode* atual = ht0->balde[indice];
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
	HashNode* atual = ht0->balde[indice];
	HashNode* anterior = NULL;
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
		HashNode* atual = ht0->balde[i];
		while(atual){
			HashNode* libera = atual;
			atual = atual->prox;
			free(libera->chave);
			free(libera);
		}
	}
	free(ht0->balde);
	free(ht0);

}

typedef struct {
	tabelaHash* ht;
	int bucket_index;
	HashNode* current_node;
} iteradorHash;

hashIterator hash_iterador(hashTable ht) {
	tabelaHash* ht0 = (tabelaHash*)ht;
	iteradorHash* it = malloc(sizeof(iteradorHash));
	if (!it) return NULL;
	it->ht = ht0;
	it->bucket_index = 0;
	it->current_node = NULL;
	while (it->bucket_index < ht0->size && ht0->balde[it->bucket_index] == NULL) {
		it->bucket_index++;
	}
	if (it->bucket_index < ht0->size) {
		it->current_node = ht0->balde[it->bucket_index];
	} else {
		it->current_node = NULL;
	}
	return (hashIterator)it;
}

bool hash_iterador_tem_proximo(hashIterator it) {
	iteradorHash* i = (iteradorHash*)it;
	return i->current_node != NULL;
}

const char* hash_iterador_proximo(hashIterator it) {
	iteradorHash* i = (iteradorHash*)it;
	if (!i->current_node) return NULL;
	const char* chave = i->current_node->chave;
	if (i->current_node->prox != NULL) {
		i->current_node = i->current_node->prox;
	} else {
		i->bucket_index++;
		while (i->bucket_index < i->ht->size && i->ht->balde[i->bucket_index] == NULL) {
			i->bucket_index++;
		}
		if (i->bucket_index < i->ht->size) {
			i->current_node = i->ht->balde[i->bucket_index];
		} else {
			i->current_node = NULL;
		}
	}
	return chave;
}

void hash_finalizar_iterador(hashIterator it) {
	free(it);
}
