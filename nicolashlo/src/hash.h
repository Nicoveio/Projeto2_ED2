#ifndef _HASH__
#define _HASH__
#include <stdbool.h>
#include <stdio.h>

/* 
Tipo opaco para a Tabela Hash, refere-se a estrutura da tabela. 
*/

typedef void* hashTable;

/*
  Tipo opaco para o iterador da tabela hash.
  Usado para percorrer todas as chaves armazenadas.
*/
typedef void* hashIterator;

/* 
Cria uma nova tabela Hash, do tamanho especificado por 'size'.
Para melhor desempenho, recomenda-se um numero primo maior que o número de elementos esperado.
Caso seja passado um tamanho x <=0, é retornado uma tabela nula.
*/

hashTable createHashTable(int size);

/* 
Insere um elemento na tabela através de um par chave-valor, a tabela fará uma cópia interna da chave e atribuirá a um valor.
Caso o valor associado já exista, seu valor associado é atualizado. Totalmente proibido passar numeros negativos.
*/
void hashPut(hashTable ht, const char*key, int value);

/*
Busca um elemento na tabela através de sua chave, retornando o estado da busca se bem-sucedida ou mal-sucedida.
Caso seja bem-sucedida, retorna a 'value' o valor encontrado.
*/

bool hashGet(hashTable ht, const char*key, int* value);

/*
Remove um elemento da tabela através de sua chave.
*/

void hashRemove(hashTable ht, const char* key);
/*
Destrói a tabela definitivamente.
*/
void hashTableDestroy(hashTable ht);

/*
  Cria um iterador para percorrer a tabela hash.
  @param ht Tabela hash a ser iterada.
  @return Um iterador para a tabela hash, ou NULL em caso de falha.
*/
hashIterator hash_iterador(hashTable ht);

/*
  Verifica se ainda existem chaves não visitadas no iterador.
  @param it Iterador da tabela hash.
  @return true se houver mais chaves para visitar; false caso contrário.
*/
bool hash_iterador_tem_proximo(hashIterator it);

/*
  Retorna a próxima chave no iterador.
  @param it Iterador da tabela hash.
  @return Ponteiro para a string da próxima chave, ou NULL se não houver mais.
*/
const char* hash_iterador_proximo(hashIterator it);

/*
  Libera os recursos usados pelo iterador.
  @param it Iterador da tabela hash a ser liberado.
*/
void hash_finalizar_iterador(hashIterator it);
#endif