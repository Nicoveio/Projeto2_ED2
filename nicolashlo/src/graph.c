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
#include <float.h>
#include <stdint.h>
#include "utils.h"

#define INFINITO DBL_MAX


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
	hashTable nomeId;
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
	while(!ehPrimo(candidato)){
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
	g->localizacaoNos = newSmuTreap(1, 1.2, 0.1);
	if(!g->nomeId ||!g->localizacaoNos ){
		free(g->vertices);
		free(g->adjacencia);
		hashTableDestroy(g->nomeId);
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
	if(g0->nosInseridos >= g0->maxNos){
		printf("Grafo cheio");
		exit(1);
	}
	int novoId = g0->nosInseridos;
	insertSmuT(g0->localizacaoNos, coord->x, coord->y,(void*)(uintptr_t)novoId, 0, NULL);
	g0->vertices[novoId].nome = duplicar_string(nome);
	g0->vertices[novoId].coord = coord;
	g0->adjacencia[novoId] = lista_cria();

	g0->nosInseridos++;
	return novoId;

}


Node getNode(Graph g, char* nome){

	if (!g || !nome)return -1;

	Grafo *g0 = (Grafo*)g;

	int idEncontrado;
	bool sucesso = hashGet(g0->nomeId, nome, &idEncontrado);
	if(sucesso == true)
		return idEncontrado;
	else  return -1;
}

Edge addEdge(Graph g, Node origem, Node destino, Info infoAresta){
	if(!g)return NULL;
	Grafo* g0 = (Grafo*) g;

	if(origem < 0 || origem>=g0->nosInseridos || destino<0 || destino >= g0->nosInseridos){
		return NULL;
	}

	Aresta* nova = malloc(sizeof(Aresta));
	if(!nova){
		printf("erro na alocação de memória");
		exit(2);
	}

	nova->destino = destino;
	nova->info = infoAresta;
	nova->habilitada = true;


	lista_insere(g0->adjacencia[origem], nova);

	return (Edge)nova;

}

void adjacentEdges(Graph g, Node node, Lista listaSaida){
	if (!g || !listaSaida || node<0)return;
	Grafo*g0 = (Grafo*)g;
	if(node<0 || node>=g0->nosInseridos)return;

	Lista listaFonte = g0->adjacencia[node];
	if(lista_vazia(listaFonte))return;


	Iterador it = lista_iterador(listaFonte);
	while(iterador_tem_proximo(it)==true){
		Aresta* atual = (Aresta*) iterador_proximo(it);
		lista_insere(listaSaida, atual);
	}
	iterador_destroi(it);
	return;
}

Node getToNode(Graph g, Edge e){
	if(!e) return -1;
	Aresta* a = (Aresta*)e;
	return a->destino;
}

Info getEdgeInfo (Graph g, Edge e){
	if(!e)return NULL;
	Aresta* a = (Aresta*)e;
	return a->info;

}

static double distanciaHeuristica(Grafo* g, Node a, Node b) {
    Coordenadas* coord_a = g->vertices[a].coord;
    Coordenadas* coord_b = g->vertices[b].coord;
    double dx = coord_b->x - coord_a->x;
    double dy = coord_b->y - coord_a->y;
    return sqrt(dx * dx + dy * dy);
}

static Lista reconstruirCaminho(Node* veioDe, Node atual) {
    Lista caminhoTotal = lista_cria();
    lista_insere(caminhoTotal, (void*)(uintptr_t)atual);
    while (veioDe[atual] != -1) {
        atual = veioDe[atual];
        lista_insere(caminhoTotal, (void*)(uintptr_t)atual); 
    }
    return caminhoTotal;
}

Lista findPath(Graph g, Node inicio, Node fim, int criterio, CalculaCustoAresta funcCusto){
	Grafo *g0 = (Grafo*)g;

	priorityQueue openSet = createPriorityQueue(g0->maxNos);
	Node *veioDe = malloc(g0->maxNos * sizeof(Node));
	double *gScore = malloc(g0->maxNos * sizeof(double));

	for(int i=0; i<g0->maxNos; i++){
		gScore[i] = INFINITO;
		veioDe[i] = -1;
	}
	gScore[inicio] = 0.0;

	double hInicio = distanciaHeuristica(g0, inicio, fim);

	pq_insert(openSet, inicio, gScore[inicio] + hInicio);
	while(pq_empty(openSet)==false){
		Node atual = pq_extract_min(openSet);

		if(atual == fim){
			Lista caminhoFinal = reconstruirCaminho(veioDe, atual);
			free(veioDe);
			free(gScore);
			pq_destroy(openSet);
			return caminhoFinal;
		}
		Lista arestasVizinhas = lista_cria();
		adjacentEdges(g, atual, arestasVizinhas);
		Iterador it = lista_iterador(arestasVizinhas);
		while( iterador_tem_proximo(it)){
			Aresta *aresta = (Aresta*)iterador_proximo(it);

			if(aresta->habilitada == false) 
				continue;
			Node vizinho = aresta->destino;
			double custoAresta = funcCusto(aresta->info, criterio);

			double gScoreTentativa = gScore[atual]+custoAresta;

			if(gScoreTentativa<gScore[vizinho]){
				veioDe[vizinho] = atual;
				gScore[vizinho] = gScoreTentativa;
				double fScoreVizinho = gScoreTentativa + distanciaHeuristica(g0, vizinho, fim);
				pq_insert(openSet, vizinho, fScoreVizinho);
			}
		}
		iterador_destroi(it);
		lista_libera(arestasVizinhas);
	}
	pq_destroy(openSet);
    free(veioDe);
    free(gScore);
    return lista_cria();
} 

void killDG(Graph g){
	if(!g)return;
	Grafo *g0 = (Grafo*)g;
	for(int i=0; i<g0->nosInseridos; i++){
		Lista listaFonte = g0->adjacencia[i];
		Iterador it = lista_iterador(listaFonte);
		while(iterador_tem_proximo(it)==true){
			Aresta *atual = (Aresta*)iterador_proximo(it);
			free(atual->info);
			free(atual);
		}
		iterador_destroi(it);
		lista_libera(listaFonte);
		free(g0->vertices[i].nome);
		free(g0->vertices[i].coord);
	}
	free(g0->vertices);
	free(g0->adjacencia);
	hashTableDestroy(g0->nomeId);
	killSmuTreap(g0->localizacaoNos);
	/* hashDestroy(g->subgrafos) */

	free(g);
}