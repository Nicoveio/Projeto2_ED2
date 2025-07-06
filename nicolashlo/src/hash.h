#ifndef _HASH__
#define _HASH__
#include <stdbool.h>
#include <stdio.h>

/* 
Tipo opaco para a Tabela Hash, refere-se a estrutura da tabela. 
*/

typedef void* hashTable;

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

#endif