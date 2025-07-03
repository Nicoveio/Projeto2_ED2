#ifndef _GRAPH__
#define _GRAPH__

#include "lista.h"

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


typedef void *Graph;
typedef int Node;
typedef void *Edge;
typedef void *Info;

#define CRITERIO_DISTANCIA 0
#define CRITERIO_TEMPO 1


/*
  Invocado quando uma aresta é "descoberta"/"percorrida"/"classificada". 
  Tambem informa os tempos de descoberta e finalizacao
 */
bool (*procEdge)(g,e,td,tf, void *extra); 

/*
  Invocado quando percurso e' recomecado
 */
bool (*dfsRestarted)(g, void *extra);



/*
    Cria um grafo com, no maximo, "nVert" vertices.
 */
Graph createGraph(nVert, bool directed);


/*
    Retorna numero maximo de vertices que grafo g pode conter.
 */
int getMaxNodes(Graph g);


/*
    Retorna numero total de vertices adicionados ao grafo gr.
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
Edge getEdge(Node g, Node from, Node to);


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
void removeEdge(g,e);


/*
    Verifica se há alguma aresta direcionada entre o nó origem e o nó destino
 */
bool isAdjacent(Graph g, Node from, Node to);


/* 
   Adiciona 'a lista "nosAdjacentes" os nos adjacentes 'a "node".
 */
void adjacentNodes(g, node, nosAdjacentes);


/*
   Adiciona 'a lista "arestaAdjacentes" as arestas (x,y), tal que,
   x == node.
 */
void adjacentEdges(g, node, Lista arestasAdjacentes);


/*
   Insere na lista "nomesNodes" os nomes atribuidos aos nos do grafo.
 */
void  getNodeNames(Graph g, Lista nomesNodes);


/*
   Insere na lista "arestas", as arestas de g.
 */
void getEdges(g, Lista arestas);

/*Modifica o estado temporariamente da aresta para 'desabilitada', sem removê-la 'fisicamente' do grafo. 
  Arestas nesse estado serão ignoradas por algoritmos de busca de caminho.*/

void disableEdge(Graph g, Edge e);

/*Modifica o estado temporariamente da aresta para 'habilitada', tornando-a 'visível' novamente para os algoritmos de busca de caminho.*/

void enableEdge(Graph g, Edge e);

/*Verifica se a aresta 'e' em um grafo 'g' está no estado 'habilitado'. */

bool isEdgeEnabled(Graph g, Edge e);

/* Calcula o melhor percurso entre nós baseado em um critério. Retorna uma lista com a sequência de nós do percurso. */

Lista findPath(Graph g, Node start, Node end, int criterio);


/*
   Faz percurso em profundidade sobre  g, a partir do no' node, classificando 
   as arestas do grafo, invocando a respectiva funcao.
      A busca em profundidade, eventualmente, pode produzir uma floresta.
   newTree e' invocada sempre que o percurso for retomado.
 */  
bool dfs(g, node, procEdge treeEdge, forwardEdge, returnEdge,
	 crossEdge, newTree, void *extra);


/*
   Percorre o grafo g em largura, a partir do no' node. discoverNode e' usada
   para a aresta (x,y) usada para "descobrir" o y.
 */
bool bfs(g, node, discoverNode, void *extra);


/*
   Destroi o grafo "g".
 */
void killDG(Graph g);

/*
 **********************
 * SUB-GRAFOS
 **********************
/*
    Calcula o subgrafo composto  pelos vertices cujos nomes estao no vetor nomesVerts
(nVerts e' o tamanho deste vetor). Caso comAresta seja true calcula o subgrafo 
induzido pelos vertices em nomesVers
 */
void  createSubgraphDG(g, char *nomeSubgrafo, char *nomesVerts[], int nVert,
		       bool comArestas);


/*
    Adiciona a aresta ao subgrafo.
 */
Edge includeEdgeSDG(g, char *nomeSubgrafo, e);

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
Graph produceGraph(g, nomeSubgrafo);


#endif