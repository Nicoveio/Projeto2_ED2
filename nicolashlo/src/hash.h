#ifndef _HASH__
#define _HASH__

typedef *void hashTable;

hashTable* createHashTable(int size);

void hashPut(hashTable ht, const char*key, int value);

void hashGet(hashTable ht, const char*key, int value);

void hashRemove(hashTable ht, const char* key);

void hashTableDestroy(hashTable ht);