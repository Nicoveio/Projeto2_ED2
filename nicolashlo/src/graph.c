#include "graph.h"
#include "smutreap.h"
#include "hash.h"
#include "lista.h"
#include "priority_queue.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
typedef struct{
	double x;
	double y;
}Coordenadas;
typedef struct{
	Node destino;
	Info info; // dados da aresta (rua que ela faz parte, etc)
	bool habilitada;
}Aresta;

typedef struct{
	char* nome;
	Coordenadas* coord;
} Vertice;

typedef struct Grafo{
	Vertice* vertices;
	Lista* adjacencia;
	int maxNos;
	int nosInseridos;
	SmuTreap localizacaoNos;
	HashTable nomeId;
} Grafo;

bool ehPrimo(int n){
	if(n<=1)return false;
	if (n==2)return true;
	if(n%2==0)return false;

	for(int i=3; (i*i)<=n; i+=2){
		if(n%i == 0)return false;
	}
	return true;
}

int proxPrimo(int n){
	int candidato = n+1;
	while(!ehprimo(candidato)){
		candidato++;
	}
	return candidato;
}

Graph createGraph(int nVert, bool directed, char* nomeGrafo){

	if(nVert <=0)
		return NULL;

	Grafo* g =(Grafo*) malloc(sizeof(Grafo));
	if(!g){
		printf("falha na alocação de memória no grafo.\n");
		exit(1);
	}

	g->maxNos = nVert;
	g->nosInseridos = 0;

	g->vertices = calloc(nVert, sizeof(Vertice));
	g->adjacencia = calloc(nVert, sizeof(Lista));
	if(!g->vertices || !g->adjacencia){
		free(g->vertices);
		free(g->adjacencia);
		free(g);
		printf("falha na alocação de memória no grafo.\n");
		exit(1);
	}
	int primo = proxPrimo(nVert);
	g->nomeId = createHashTable(primo);
	g->localizacaoNos = newSmutreap(1, 1.2, 0.1);
	if(!g->nomeId ||!g->localizacaoNos ){
		free(g->vertices);
		free(g->adjacencia);
		hashDestroy(g->nomeId);
		killSmuTreap(g->localizacaoNos);
		free(g);
		printf("falha na alocação de memória no grafo.\n");
		exit(1);
	}
	return (Graph)g;
}

int getMaxNodes(Graph g){
	Grafo *g0 = (Grafo*)g;
	return g0->maxNos;
}

int getTotalNodes(Graph g){
	Grafo *g0 = (Grafo*)g;
	return g0->nosInseridos;
}

Node addNode(Graph g, char*nome, Info info){
	if(!g || !nome || !info) exit(1);
    Grafo* g0 = (Grafo*)g;
    Coordenadas *coord = (Coordenadas*)info;
    int valor;
	bool verificador = hashGet(g0->nomeId, nome, &valor);
	if(verificador==true){
		free(info);
		return valor; 
	}
	if(g0->nosInseridos >= g->maxNos){
		printf("Grafo cheio");
		exit(1);
	}
	int novoId = g0->nosInseridos;
	insertSmuT(g0->localizacaoNos, coord->x, coord->y,(void*)(uintptr_t)novoId, 0, NULL);
	g0->vertices[novoId].nome = strdup(nome);
	g0->vertices[novoId].coord = coord;
	g0->adjacencia[novoId] = lista_cria();

	g0->nosInseridos++;
	return novoId;


}