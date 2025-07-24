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
	if(!g->nomeId ||!g->localizacaoNos || !g->subgrafos ){
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
        // Uma aresta nula não pode estar habilitada.
        return false;
    }
    // Faz o cast do ponteiro opaco 'Edge' para a struct interna 'Aresta'.
    Aresta* a = (Aresta*)e;

    // Retorna o valor booleano do campo 'habilitada'.
    return a->habilitada;
}

void incomingEdges(Graph g, Node node, Lista arestasEntrada) {
    if (!g || !arestasEntrada || node < 0) return;
    Grafo* g0 = (Grafo*)g;
    if (node >= g0->nosInseridos) return;

    // Percorre TODOS os nós para ver qual deles tem uma aresta para o 'node'
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
        return -1; // Retorna um ID inválido se a entrada for nula
    }
    Grafo* g0 = (Grafo*)g;

    // A única forma de encontrar a origem é percorrer as listas de adjacência
    // para ver qual delas contém a aresta 'e'.
    for (Node i = 0; i < g0->nosInseridos; i++) {
        Lista arestas_saindo = g0->adjacencia[i];
        if (lista_vazia(arestas_saindo)) {
            continue;
        }

        Iterador it = lista_iterador(arestas_saindo);
        while (iterador_tem_proximo(it)) {
            // Pega o ponteiro da aresta na lista
            Edge aresta_atual = (Edge)iterador_proximo(it);
            
            // Compara o ENDEREÇO de memória dos ponteiros
            if (aresta_atual == e) {
                // Se encontramos a aresta 'e' na lista de adjacência do nó 'i',
                // então 'i' é o nó de origem.
                iterador_destroi(it);
                return i;
            }
        }
        iterador_destroi(it);
    }

    return -1; // Retorna -1 se a aresta não for encontrada em nenhuma lista
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
	hashTableDestroy(g0->subgrafos);

	free(g);
}

static Subgrafo* getSubgraph(Grafo* g, const char* nomeSubgrafo) {
    Subgrafo* sg;
    int sg_ptr_int;
    if (hashGet(g->subgrafos, nomeSubgrafo, &sg_ptr_int)) {
        sg = (Subgrafo*)(uintptr_t)sg_ptr_int;
        return sg;
    }
    return NULL;
}

void createSubgraphDG(Graph g_opaco, char* nomeSubgrafo, char* nomesVerts[], int nVert, bool comArestas) {
    Grafo* g = (Grafo*)g_opaco;

    Subgrafo* sg = malloc(sizeof(Subgrafo));
    sg->nome = duplicar_string(nomeSubgrafo);
    sg->nos = createHashTable(proxPrimo(nVert));
    sg->arestas = createHashTable(proxPrimo(nVert * 4));

    hashPut(g->subgrafos, nomeSubgrafo, (int)(uintptr_t)sg);

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