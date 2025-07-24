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

#define BRANCO 0
#define CINZA 1
#define PRETO 2

#define INFINITO DBL_MAX
#define MAX_SUBGRAFOS 50

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

typedef struct {
    char* nome;
    hashTable nos;   
    hashTable arestas; 
} Subgrafo;

typedef struct Grafo{
	Vertice* vertices;
	Lista* adjacencia;
	int maxNos;
	int nosInseridos;
	SmuTreap localizacaoNos;
	hashTable nomeId;
	hashTable subgrafos;
    Subgrafo** vestiario_de_subgrafos; 
    int contador_subgrafos;
} Grafo;

static Coordenadas* coordenadas_para_callback = NULL;



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
    setPrioridadeMax(g->localizacaoNos, 10000);
    g->subgrafos = createHashTable(13); 
    g->vestiario_de_subgrafos = calloc(MAX_SUBGRAFOS, sizeof(Subgrafo*));
	if(!g->nomeId ||!g->localizacaoNos || !g->subgrafos || !g->vestiario_de_subgrafos ){
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


void boundingBoxVertice(DescritorTipoInfo tipo, Info i, double *x, double *y, double *w, double *h) {
    if (coordenadas_para_callback) {
        *x = coordenadas_para_callback->x;
        *y = coordenadas_para_callback->y;
    } else {
        *x = 0;
        *y = 0;
    }
    *w = 0;
    *h = 0;
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
	coordenadas_para_callback = coord;
  
    insertSmuT(g0->localizacaoNos, coord->x, coord->y, (void*)(uintptr_t)novoId, 0, &boundingBoxVertice);

	g0->vertices[novoId].nome = duplicar_string(nome);
	g0->vertices[novoId].coord = coord;
	g0->adjacencia[novoId] = lista_cria();

    hashPut(g0->nomeId, nome, novoId);

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

Info getNodeInfo(Graph g, Node node) {
    if (!g)
        return NULL;
    Grafo* g0 = (Grafo*)g;
    if (node < 0 || node >= g0->nosInseridos) 
        return NULL;
    return (Info)g0->vertices[node].coord;
}
char* getNodeName(Graph g, Node node) {
    if (!g) {
        return NULL;
    }
    Grafo* g0 = (Grafo*)g;
    if (node < 0 || node >= g0->nosInseridos) 
        return NULL;
    return g0->vertices[node].nome;
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

Edge getEdge(Graph g, Node from, Node to) {
    if (!g) return NULL;
    Grafo* g0 = (Grafo*)g;

    if (from < 0 || from >= g0->nosInseridos || to < 0 || to >= g0->nosInseridos) 
        return NULL;
    Lista arestas_vizinhas = g0->adjacencia[from];
    if (lista_vazia(arestas_vizinhas)) 
        return NULL;
    Iterador it = lista_iterador(arestas_vizinhas);
    while (iterador_tem_proximo(it)) {
        Aresta* aresta_atual = (Aresta*)iterador_proximo(it);
        if (aresta_atual->destino == to) {
            iterador_destroi(it);
            return (Edge)aresta_atual;
        }
    }

    iterador_destroi(it);
    return NULL; 
}

bool getNodesInRegion(Graph g, double x1, double y1, double x2, double y2, Lista nosEncontrados) {
    if (!g || !nosEncontrados) return false;
    
    Grafo* g0 = (Grafo*)g;
    return getNodesDentroRegiaoSmuT(g0->localizacaoNos, x1, y1, x2, y2, nosEncontrados);
}

Node getNodeIdFromSmuTNode(Graph g, NodeSmu smuNode) {
    if (!g || !smuNode) return -1;
    
    Grafo* g0 = (Grafo*)g;
    Info id_info = getInfoSmuT(g0->localizacaoNos, smuNode);
    return (Node)(uintptr_t)id_info;
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

Node findNearestNode(Graph g, double x, double y) {
    Grafo* g0 = (Grafo*)g;
    Lista nos_encontrados = lista_cria();
    Node no_mais_proximo = -1;
    
    double raio_busca = 50.0;
    bool encontrou_algum_no = false;

    while (!encontrou_algum_no) {
        double x1 = x - raio_busca;
        double y1 = y - raio_busca;
        double x2 = x + raio_busca;
        double y2 = y + raio_busca;


        encontrou_algum_no = getNodesDentroRegiaoSmuT(g0->localizacaoNos, x1, y1, x2, y2, nos_encontrados);

        if (!encontrou_algum_no) {
            raio_busca *= 2;
        }
    }

    double menor_dist_quadrada = DBL_MAX;

    Iterador it = lista_iterador(nos_encontrados);
    while (iterador_tem_proximo(it)) {

        NodeSmu no_da_arvore = (NodeSmu)iterador_proximo(it);
        Info id_info = getInfoSmuT(g0->localizacaoNos, no_da_arvore);
        Node id_atual = (Node)(uintptr_t)id_info;
        Coordenadas* coord_no = (Coordenadas*)getNodeInfo(g, id_atual);
        
        double dx = coord_no->x - x;
        double dy = coord_no->y - y;
        double dist_quadrada = dx * dx + dy * dy;

        if (dist_quadrada < menor_dist_quadrada) {
            menor_dist_quadrada = dist_quadrada;
            no_mais_proximo = id_atual;
        }
    }
    iterador_destroi(it);
    lista_libera(nos_encontrados);

    return no_mais_proximo;
}


void disableEdge(Graph g, Edge e) {
    if (!e) return;
    Aresta* a = (Aresta*)e;
    a->habilitada = false;
}

void enableEdge(Graph g, Edge e) {
    if (!e) return;
    Aresta* a = (Aresta*)e;
    a->habilitada = true;
}


bool isEdgeEnabled(Graph g, Edge e) {
    if (!e) {
        return false;
    }

    Aresta* a = (Aresta*)e;

    return a->habilitada;
}

void incomingEdges(Graph g, Node node, Lista arestasEntrada) {
    if (!g || !arestasEntrada || node < 0) return;
    Grafo* g0 = (Grafo*)g;
    if (node >= g0->nosInseridos) return;

    for (Node i = 0; i < g0->nosInseridos; i++) {
        Lista arestas_saindo = g0->adjacencia[i];
        if (lista_vazia(arestas_saindo)) continue;

        Iterador it = lista_iterador(arestas_saindo);
        while (iterador_tem_proximo(it)) {
            Aresta* a = (Aresta*)iterador_proximo(it);
            if (a->destino == node) {
                lista_insere(arestasEntrada, (Edge)a);
            }
        }
        iterador_destroi(it);
    }
}

Node getFromNode(Graph g, Edge e) {
    if (!g || !e) {
        return -1; 
    }
    Grafo* g0 = (Grafo*)g;


    for (Node i = 0; i < g0->nosInseridos; i++) {
        Lista arestas_saindo = g0->adjacencia[i];
        if (lista_vazia(arestas_saindo)) {
            continue;
        }

        Iterador it = lista_iterador(arestas_saindo);
        while (iterador_tem_proximo(it)) {
            // Pega o ponteiro da aresta na lista
            Edge aresta_atual = (Edge)iterador_proximo(it);
            

            if (aresta_atual == e) {
                iterador_destroi(it);
                return i;
            }
        }
        iterador_destroi(it);
    }

    return -1;
}

void setNodeInfo(Graph g, Node node, Info info) {
    Grafo* g0 = (Grafo*) g;
    g0->vertices[node].nome = info;  
}


void setEdgeInfo(Graph g, Edge e, Info info) {
    Aresta* a = (Aresta*) e;
    a->info = info;
}

void removeEdge(Graph g, Edge e) {
    Aresta* a = (Aresta*) e;
    a->habilitada = false;
}


bool isAdjacent(Graph g, Node from, Node to) {
    Grafo* g0 = (Grafo*) g;
    Lista adj = g0->adjacencia[from];
    Iterador it = lista_iterador(adj);
    while (iterador_tem_proximo(it)) {
        Aresta* a = (Aresta*) iterador_proximo(it);
        if (a->destino == to && a->habilitada) {
            iterador_destroi(it);
            return true;
        }
    }
    iterador_destroi(it);
    return false;
}


void adjacentNodes(Graph g, Node node, Lista nosAdjacentes) {
    Grafo* g0 = (Grafo*) g;
    Lista adj = g0->adjacencia[node];
    Iterador it = lista_iterador(adj);
    while (iterador_tem_proximo(it)) {
        Aresta* a = (Aresta*) iterador_proximo(it);
        if (a->habilitada) {
            lista_insere(nosAdjacentes, (void*)(intptr_t)a->destino);
        }
    }
    iterador_destroi(it);
}


void getNodeNames(Graph g, Lista nomesNodes) {
    Grafo* g0 = (Grafo*) g;
    for (int i = 0; i < g0->nosInseridos; i++) {
        lista_insere(nomesNodes, g0->vertices[i].nome);
    }
}


void getEdges(Graph g, Lista arestas) {
    Grafo* g0 = (Grafo*) g;
    for (int i = 0; i < g0->maxNos; i++) {
        Lista adj = g0->adjacencia[i];
        Iterador it = lista_iterador(adj);
        while (iterador_tem_proximo(it)) {
            lista_insere(arestas, iterador_proximo(it));
        }
        iterador_destroi(it);
    }
}

static void dfs_visit(Graph g, int u, int *cor,
                      procEdgeCallback treeEdge,
                      procEdgeCallback forwardEdge,
                      procEdgeCallback returnEdge,
                      procEdgeCallback crossEdge,
                      void *extra) {
    Grafo *g0 = (Grafo *)g;
    cor[u] = CINZA;

    Iterador it = lista_iterador(g0->adjacencia[u]);
    while (iterador_tem_proximo(it)) {
        Aresta *a = (Aresta *)iterador_proximo(it);
        int v = a->destino;

        if (cor[v] == BRANCO) {
            if (treeEdge) treeEdge(g, (Edge)a, 0, 0, extra);  // passa a aresta e ints placeholder
            dfs_visit(g, v, cor, treeEdge, forwardEdge, returnEdge, crossEdge, extra);
        } else if (cor[v] == CINZA) {
            if (returnEdge) returnEdge(g, (Edge)a, 0, 0, extra);
        } else {
            if (crossEdge) crossEdge(g, (Edge)a, 0, 0, extra);
        }
    }
    iterador_destroi(it);
    cor[u] = PRETO;
}

bool dfs(Graph g, Node node,
         procEdgeCallback treeEdge,
         procEdgeCallback forwardEdge,
         procEdgeCallback returnEdge,
         procEdgeCallback crossEdge,
         dfsRestartedCallback newTree,
         void *extra) {
    Grafo *g0 = (Grafo *)g;

    int *cor = calloc(g0->maxNos, sizeof(int));

    for (int i = 0; i < g0->nosInseridos; i++) {
        cor[i] = BRANCO;
    }

    for (int i = 0; i < g0->nosInseridos; i++) {
        if (cor[i] == BRANCO) {
            if (newTree) newTree(g, extra); 
            dfs_visit(g, i, cor, treeEdge, forwardEdge, returnEdge, crossEdge, extra);
        }
    }

    free(cor);
    return true;
}

bool bfs(Graph g, Node node, procEdgeCallback discoverNode, void *extra) {
    Grafo *g0 = (Grafo *)g;

    int *cor = calloc(g0->maxNos, sizeof(int));
    if (!cor) return false;

    for (int i = 0; i < g0->nosInseridos; i++) {
        cor[i] = BRANCO;
    }

    Fila fila = fila_cria();
    if (!fila) {
        free(cor);
        return false;
    }

    cor[node] = CINZA;
    fila_insere(fila, (Elemento)(intptr_t)node);

    while (!fila_vazia(fila)) {
        int u = (int)(intptr_t)fila_remove(fila);

        Iterador it = lista_iterador(g0->adjacencia[u]);
        while (iterador_tem_proximo(it)) {
            Aresta *a = (Aresta *)iterador_proximo(it);
            int v = a->destino;

            if (cor[v] == BRANCO) {
                cor[v] = CINZA;
                fila_insere(fila, (Elemento)(intptr_t)v);

                if (discoverNode) {
                    discoverNode(g, (Edge)a, 0, 0, extra);
                }
            }
        }
        iterador_destroi(it);
        cor[u] = PRETO;
    }

    fila_libera(fila);
    free(cor);
    return true;
}




void killDG(Graph g) {
    if (!g) return;
    Grafo* g0 = (Grafo*)g;
    for (int i = 0; i < g0->nosInseridos; i++) {
        Lista listaFonte = g0->adjacencia[i];
        if (listaFonte) {
            Iterador it = lista_iterador(listaFonte);
            while (iterador_tem_proximo(it)) {
                Aresta* atual = (Aresta*)iterador_proximo(it);
                if (atual->info) free(atual->info); 
                free(atual);                     
            }
            iterador_destroi(it);
            lista_libera(listaFonte); 
        }
        
        if (g0->vertices[i].nome) free(g0->vertices[i].nome);
        if (g0->vertices[i].coord) free(g0->vertices[i].coord);
    }

    for (int i = 0; i < g0->contador_subgrafos; i++) {
        Subgrafo* sg = g0->vestiario_de_subgrafos[i];
        if (sg) {
            free(sg->nome);
            hashTableDestroy(sg->nos);
            hashTableDestroy(sg->arestas);
            free(sg);
        }
    }
    
    free(g0->vestiario_de_subgrafos);
    hashTableDestroy(g0->subgrafos);
    free(g0->vertices);
    free(g0->adjacencia);
    hashTableDestroy(g0->nomeId);
    killSmuTreap(g0->localizacaoNos);

    free(g0);
}

static Subgrafo* getSubgraph(Grafo* g, const char* nomeSubgrafo) {
    int id_subgrafo;
    if (hashGet(g->subgrafos, nomeSubgrafo, &id_subgrafo)) {
       return g->vestiario_de_subgrafos[id_subgrafo];
    }
    return NULL;
}

void createSubgraphDG(Graph g_opaco, char* nomeSubgrafo, char* nomesVerts[], int nVert, bool comArestas) {
    Grafo* g = (Grafo*)g_opaco;

    Subgrafo* sg = malloc(sizeof(Subgrafo));
    sg->nome = duplicar_string(nomeSubgrafo);
    sg->nos = createHashTable(proxPrimo(nVert));
    sg->arestas = createHashTable(proxPrimo(nVert * 4));

    int id_subgrafo = g->contador_subgrafos;
    g->vestiario_de_subgrafos[id_subgrafo] = sg;
    hashPut(g->subgrafos, nomeSubgrafo, id_subgrafo); // Guarda o índice (int)
    g->contador_subgrafos++;
    
    for (int i = 0; i < nVert; i++) {
        Node id_no = getNode(g_opaco, nomesVerts[i]);
        if (id_no != -1) {
            hashPut(sg->nos, nomesVerts[i], id_no);
        }
    }

    if (comArestas) {
        for (int i = 0; i < nVert; i++) {
            Node id_origem = getNode(g_opaco, nomesVerts[i]);
            if (id_origem == -1) continue;

            Lista arestas_vizinhas = g->adjacencia[id_origem];
            if (lista_vazia(arestas_vizinhas)) continue;

            Iterador it = lista_iterador(arestas_vizinhas);
            while (iterador_tem_proximo(it)) {
                Aresta* aresta = (Aresta*)iterador_proximo(it);
                Node id_destino = aresta->destino;

                int valor_qualquer;
                if (hashGet(sg->nos, getNodeName(g_opaco, id_destino), &valor_qualquer)) {
                    includeEdgeSDG(g_opaco, nomeSubgrafo, (Edge)aresta);
                }
            }
            iterador_destroi(it);
        }
    }
}

Edge includeEdgeSDG(Graph g_opaco, char* nomeSubgrafo, Edge e_opaco) {
    Grafo* g = (Grafo*)g_opaco;
    Subgrafo* sg = getSubgraph(g, nomeSubgrafo);
    if (!sg) return NULL;

    char chave_ponteiro[32];
    sprintf(chave_ponteiro, "%p", e_opaco);

    hashPut(sg->arestas, chave_ponteiro, 1);
    return e_opaco;
}

bool existsEdgeSDG(Graph g_opaco, char* nomeSubgrafo, Edge e_opaco) {
    Grafo* g = (Grafo*)g_opaco;
    Subgrafo* sg = getSubgraph(g, nomeSubgrafo);
    if (!sg) return false;

    char chave_ponteiro[32];
    sprintf(chave_ponteiro, "%p", e_opaco);

    int valor_qualquer;
    return hashGet(sg->arestas, chave_ponteiro, &valor_qualquer);
}

void excludeEdgeSDG(Graph g_opaco, char* nomeSubgrafo, Edge e_opaco) {
    Grafo* g = (Grafo*)g_opaco;
    Subgrafo* sg = getSubgraph(g, nomeSubgrafo);
    if (!sg) return;

    char chave_ponteiro[32];
    sprintf(chave_ponteiro, "%p", e_opaco);
    hashRemove(sg->arestas, chave_ponteiro);
}

void adjacentEdgesSDG(Graph g_opaco, char* nomeSubgrafo, Node node, Lista arestasAdjacentes) {
    Grafo* g = (Grafo*)g_opaco;
    Subgrafo* sg = getSubgraph(g, nomeSubgrafo);
    if (!sg) return;

    Lista arestas_totais = g->adjacencia[node];
    if (lista_vazia(arestas_totais)) return;

    Iterador it = lista_iterador(arestas_totais);
    while (iterador_tem_proximo(it)) {
        Aresta* aresta_atual = (Aresta*)iterador_proximo(it);
        
        if (existsEdgeSDG(g_opaco, nomeSubgrafo, (Edge)aresta_atual)) {
            lista_insere(arestasAdjacentes, aresta_atual);
        }
    }
    iterador_destroi(it);
}

void getAllNodesSDG(Graph g, char *nomeSubgrafo, Lista lstNodes) {
    Grafo* g0 = (Grafo*) g;
    int sg_val = 0;
    if (!hashGet(g0->subgrafos, nomeSubgrafo, &sg_val)) return;
    Subgrafo* sg = (Subgrafo*)(intptr_t) sg_val;

    hashIterator it = hash_iterador(sg->nos);
    while (hash_iterador_tem_proximo(it)) {
        const char* nome = hash_iterador_proximo(it);
        int no_id;
        if (hashGet(g0->nomeId, nome, &no_id)) {
            lista_insere(lstNodes, (void*)(intptr_t) no_id);
        }
    }
    hash_finalizar_iterador(it);
}

void getAllEdgesSDG(Graph g, char *nomeSubgrafo, Lista lstEdges) {
    Grafo* g0 = (Grafo*) g;
    int sg_val = 0;
    if (!hashGet(g0->subgrafos, nomeSubgrafo, &sg_val)) return;
    Subgrafo* sg = (Subgrafo*)(intptr_t) sg_val;

    hashIterator it = hash_iterador(sg->arestas);
    while (hash_iterador_tem_proximo(it)) {
        const char* id = hash_iterador_proximo(it);
        int edge_val;
        if (hashGet(sg->arestas, id, &edge_val)) {
            Edge e = (Edge)(intptr_t) edge_val;
            lista_insere(lstEdges, e);
        }
    }
    hash_finalizar_iterador(it);
}

void incomingEdgesSDG(Graph g, char *nomeSubgrafo, Node node, Lista arestasEntrada) {
    Grafo* g0 = (Grafo*) g;
    int sg_val = 0;
    if (!hashGet(g0->subgrafos, nomeSubgrafo, &sg_val)) return;
    Subgrafo* sg = (Subgrafo*)(intptr_t) sg_val;

    for (int i = 0; i < g0->maxNos; i++) {
        Lista adj = g0->adjacencia[i];
        Iterador it = lista_iterador(adj);
        while (iterador_tem_proximo(it)) {
            Aresta* a = (Aresta*) iterador_proximo(it);
            int no_id;
            if (a->destino == node && hashGet(sg->nos, g0->vertices[i].nome, &no_id)) {
                lista_insere(arestasEntrada, a);
            }
        }
        iterador_destroi(it);
    }
}

Graph produceGraph(Graph g, char* nomeSubgrafo) {
    Grafo* g0 = (Grafo*)g;
    int sg_val = 0;
    if (!hashGet(g0->subgrafos, nomeSubgrafo, &sg_val)) return NULL;
    Subgrafo* sg = (Subgrafo*)(intptr_t) sg_val;

    Graph novoGrafo = createGraph(g0->maxNos, false, NULL);
    if (!novoGrafo) return NULL;

    // Adiciona os nós do subgrafo
    hashIterator itNos = hash_iterador(sg->nos);
    while (hash_iterador_tem_proximo(itNos)) {
        const char* nomeNo = hash_iterador_proximo(itNos);
        int no_id;
        if (hashGet(g0->nomeId, nomeNo, &no_id)) {
            Coordenadas* coord = g0->vertices[no_id].coord;
            addNode(novoGrafo, (char*)nomeNo, (Info)coord);
        }
    }
    hash_finalizar_iterador(itNos);

    hashIterator itArestas = hash_iterador(sg->arestas);
    while (hash_iterador_tem_proximo(itArestas)) {
        const char* chaveAresta = hash_iterador_proximo(itArestas);

        char origemNome[256];
        sscanf(chaveAresta, "%[^-]", origemNome);

        int origemId = getNode(g0, origemNome);
        if (origemId < 0) continue; 

        int edge_val;
        if (hashGet(sg->arestas, chaveAresta, &edge_val)) {
            Edge e = (Edge)(intptr_t)edge_val;
            Aresta* a = (Aresta*)e;
            Node destino = a->destino;
            Info infoAresta = a->info;

            addEdge(novoGrafo, origemId, destino, infoAresta);
        }
    }
    hash_finalizar_iterador(itArestas);

    return novoGrafo;
}
