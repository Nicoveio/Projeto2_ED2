#ifndef _GRAPH__
#define _GRAPH__

#include "lista.h"
#include "stdbool.h"

/*
Um Grafo  G e' constituido por um conjunto de vertices V e 
um conjunto de arestas E, denotado por G=(V,E). 

Pode-se associar um dado a arestas e a vertices de G.
A cada vertice e' associado um nome.

Podem ser definidos subgrafos de um dado grafo. Um subgrafo pode ser entendido 
como uma visão (ou filtro) sobre o grafo original. Ou, ainda, um subgrafo 
define (delimita) uma "região" do grafo a ser acessada.

Cada subgrafo possui um nome. Um grafo pode pode possuir vários subgrafos. 
Arestas podem ser acrescentas ou retiradas do subgrafo
Existem tambem outras operacoes que possibilitam percorrer o subgrafo,
invocando funcoes quando arestas do grafo 
sao "descobertas", podendo, inclusive, interromper o percurso.

Este modulo faz parte de um trabalho de Estrutura de Daddos do 
curso de Ciencia da Computacao da Universidade Estadual de Londrina.
E' expressamente proibido o uso de ferramentas de IA para a sua implementacao.
A documentacao deste modulo deve ser melhorada.
*/

/*
 Ponteiro opaco para a estrutura do grafo. 
 O uso de 'void *' esconde os detalhes da implementação, uma prática
 [cite_start]de encapsulamento conhecida como "Information Hiding"[cite: 281].
 */
typedef void *Graph;

/*
  Define um 'Nó' (ou vértice) do grafo como um número inteiro.
  É usado como um identificador único para cada vértice.
 */
typedef int Node;

/*
  Ponteiro opaco para a estrutura de uma aresta.
 */
typedef void *Edge;

/*
  Ponteiro genérico usado para associar qualquer tipo de informação
  customizada a um nó ou a uma aresta do grafo.
 */
typedef void *Info;


// Constantes usadas para o critério de busca de caminho na função findPath.
#define CRITERIO_DISTANCIA 0 // Para o caminho mais curto
#define CRITERIO_TEMPO 1     // Para o caminho mais rápido


/*
  Define um tipo de função de callback para ser usada quando uma aresta é
  processada durante um percurso no grafo (como DFS ou BFS).
 */
typedef bool (*procEdgeCallback)(Graph g, Edge e, int td, int tf, void *extra); 

/**
  Define um tipo de função de callback para ser usada quando um percurso 
  DFS reinicia a busca a partir de um novo vértice (formando uma nova árvore).
 */
typedef bool (*dfsRestartedCallback)(Graph g, void *extra);

/*
   Calcula o custo (ou "peso") de uma aresta com base em um critério.
   Esta função atua como um "callback" para o algoritmo findPath,
   permitindo que a busca de caminho seja genérica e independente
   dos dados específicos da aplicação (como comprimento e velocidade).
*/
typedef double (*CalculaCustoAresta)(Info info_aresta, int criterio);


/*
    Cria um grafo com, no maximo, "nVert" vertices.
 */
Graph createGraph(int nVert, bool directed, char* nomeGrafo);


/*
    Retorna numero maximo de vertices que grafo g pode conter.
 */
int getMaxNodes(Graph g);


/*
    Retorna numero total de vertices adicionados ao grafo g.
 */
int getTotalNodes(Graph g);


/*
    Adiciona um novo vértice ao grafo "g" com o nome "nome".
 */
Node addNode(Graph g, char* nome, Info info);


/*
    Retorna no' cujo de nome e' "nome". 
 */
Node getNode(Graph g, char* nome);


/*
    Retorna a informação associada ao nó do grafo g.
 */
Info getNodeInfo(Graph g, Node node);


/*
    Retorna o nome associado ao nó do grafo g.
 */
char *getNodeName(Graph g, Node node);


/*
   Insere a informação 'info' ao nó Node do grafo g.
 */
void setNodeInfo(Graph g, Node node, Info info);


/*
   Adicionar uma aresta entre um nó origem e um nó destino.
 */
Edge addEdge( Graph g, Node from, Node to, Info info);


/* 
    Retorna a aresta associada a um nó origem e a um nó destino.

 */
Edge getEdge(Graph g, Node from, Node to);


/*
     Retorna o nó origem da aresta 'e'.
 */
Node getFromNode(Graph g, Edge e);

  
/*
    Retorna o nó destino da aresta 'e'.
 */  
Node getToNode(Graph g, Edge e);


/*
    Retorna a informação associada a aresta 'e'.
 */
Info getEdgeInfo(Graph g, Edge e);


/*
   Adicionar a informação 'info' a aresta 'e'.
 */
void setEdgeInfo(Graph g, Edge e, Info info);


/*
   Remove a aresta 'e', é uma remoção com processo destrutivo permanente.
 */
void removeEdge(Graph g, Edge e);


/*
    Verifica se há alguma aresta direcionada entre o nó origem e o nó destino
 */
bool isAdjacent(Graph g, Node from, Node to);


/* 
   Adiciona 'a lista "nosAdjacentes" os nos adjacentes 'a "node".
 */
void adjacentNodes(Graph g, Node node, Lista nosAdjacentes);


/*
   Adiciona 'a lista "arestaAdjacentes" as arestas (x,y), tal que,
   x == node.
 */
void adjacentEdges(Graph g, Node node, Lista arestasAdjacentes);


/*
   Insere na lista "nomesNodes" os nomes atribuidos aos nos do grafo.
 */
void  getNodeNames(Graph g, Lista nomesNodes);


/*
   Insere na lista "arestas", as arestas de g.
 */
void getEdges(Graph g, Lista arestas);

/*Modifica o estado temporariamente da aresta para 'desabilitada', sem removê-la 'fisicamente' do grafo. 
  Arestas nesse estado serão ignoradas por algoritmos de busca de caminho.*/

void disableEdge(Graph g, Edge e);

/*Modifica o estado temporariamente da aresta para 'habilitada', tornando-a 'visível' novamente para os algoritmos de busca de caminho.*/

void enableEdge(Graph g, Edge e);

/*Verifica se a aresta 'e' em um grafo 'g' está no estado 'habilitado'. */

bool isEdgeEnabled(Graph g, Edge e);

/* Calcula o melhor percurso entre nós baseado em um critério. Retorna uma lista com a sequência de nós do percurso. */

Lista findPath(Graph g, Node start, Node end, int criterio, CalculaCustoAresta funcCusto);


/*
   Faz percurso em profundidade sobre  g, a partir do no' node, classificando 
   as arestas do grafo, invocando a respectiva funcao.
      A busca em profundidade, eventualmente, pode produzir uma floresta.
   newTree e' invocada sempre que o percurso for retomado.
 */  
bool dfs(Graph g, Node node, procEdgeCallback treeEdge, procEdgeCallback forwardEdge, procEdgeCallback returnEdge,
	 procEdgeCallback crossEdge, dfsRestartedCallback newTree, void *extra);


/*
   Percorre o grafo g em largura, a partir do no' node. discoverNode e' usada
   para a aresta (x,y) usada para "descobrir" o y.
 */
bool bfs(Graph g, Node node, procEdgeCallback discoverNode, void *extra);

/*
   Encontra o vértice do grafo cuja âncora (coordenada x,y) é a mais
   próxima de um ponto geográfico arbitrário. A busca é otimizada
   pela árvore espacial interna para ser mais eficiente que uma
   verificação linear de todos os nós.
*/
Node findNearestNode(Graph g, double x, double y);


/*
   Destroi o grafo "g".
 */
void killDG(Graph g);

/*
 **********************
 * SUB-GRAFOS
 **********************
    Calcula o subgrafo composto  pelos vertices cujos nomes estao no vetor nomesVerts
(nVerts e' o tamanho deste vetor). Caso comAresta seja true calcula o subgrafo 
induzido pelos vertices em nomesVers
 */
void  createSubgraphDG(Graph g, char *nomeSubgrafo, char *nomesVerts[], int nVert,
		       bool comArestas);


/*
    Adiciona a aresta ao subgrafo.f
 */
Edge includeEdgeSDG(Graph g, char *nomeSubgrafo, Edge e);

/*
  Retorna verdadeiro se a aresta "e" pertence ao subgrafo "nomeSubgrafo" do grafo g; 
  falso, caso contrario.
 */
bool existsEdgeSDG(Graph g, char *nomeSubgrafo, Edge e);

/*
  Retira a aresta "e" do subgrafo "nomeSubgrafo". Ou seja, desfaz a correspondente 
  operacao includeEdgeSg previamente executada. 
  Note que a aresta  "e" NAO e' removida do grafo g.
 */
void excludeEdgeSDG(Graph g, char *nomeSubgrafo, Edge e);

/*
   Adiciona 'a lista "arestaAdjacentes" as arestas (x,y), tal que:
   x == node; x pertence ao subgrafo "nomeSubgrafo", (x,y) tambem e' aresta
   do subgrafo.
 */
void adjacentEdgesSDG(Graph g, char *nomeSubgrafo, Node node, Lista arestasAdjacentes);

/*
   Adiciona 'a lista "lstNodes" (Lista<Node>) os nós do subgrafo "nomeSubgrafo".
 */
void getAllNodesSDG(Graph g, char *nomeSubgrafo,  Lista lstNodes);

/*
   Adiciona 'a lista "lstEdges" (Lista<Edge>) as arestas do subgrafo "nomeSubgrafo".
 */
void getAllEdgesSDG(Graph g, char *nomeSubgrafo, Lista lstEdges);

/*
  Adiciona à lista "arestasEntrada" as arestas (x,y), tal que:
  y == node; y pertence ao subgrafo "nomeSubgrafo", e (x,y) também é uma aresta
  do subgrafo.
*/
void incomingEdgesSDG(Graph g, char *nomeSubgrafo, Node node, Lista arestasEntrada);



/*
  Novo grafo.
 */
Graph produceGraph(Graph g, char* nomeSubgrafo);


#endif
