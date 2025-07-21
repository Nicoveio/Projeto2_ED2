#include "smutreap.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct boundingBox{
    double x1, x2, y1, y2;
}boundingBox;
 

typedef struct NodeST{
	double x,y;
	int prioridade;
	Info info;
	DescritorTipoInfo descritor;
	int hits;
	struct NodeST *dir, *esq;
	boundingBox *bb;

}NodeST;

typedef struct SmuTreapImp{
	NodeST *raiz;
	int hitCount;
	double promotionRate;
	double epsilon;
    int prioridade_max;

}SmuTreapImp;

NodeST* rotateLeft(NodeST* node);
NodeST* rotateRight(NodeST* node);
NodeST* insertSmuTAux(SmuTreap t, NodeST *atual, double x, double y, double epsilon, Info i, DescritorTipoInfo d, FCalculaBoundingBox fCalcBb);
void atualizaBoundingBox(NodeST* node);
NodeST* promoteNodeSmuAux(NodeST* raiz, NodeST* n, double epsilon);
NodeSmu getNodeSmuAux(NodeST*raiz, SmuTreapImp*t, double x, double y);
NodeST* removeNoSmuTAux(NodeST*raiz, NodeST*no, double epsilon);
bool intersecta(boundingBox* bb, double x1, double y1, double x2, double y2);
void buscaNosDentro(SmuTreap t, NodeST* no,   double x1, double y1, double x2, double y2, Lista L, bool* encontrou);
bool buscaInfosDentro(NodeST* no, SmuTreap t, double x1, double y1, double x2, double y2,
                      FdentroDeRegiao f, Lista L);
bool atingidosAux(SmuTreapImp* t, NodeST* no, double x, double y, FpontoInternoAInfo f, Lista L, bool* encontrou);
void printDotAux(FILE* fp, NodeST* no);



SmuTreap newSmuTreap(int hitCount, double promotionRate, double epsilon){
	if (hitCount<1 || promotionRate<=0.00)return NULL;
	SmuTreapImp *arvore = (SmuTreapImp*)malloc(sizeof(SmuTreapImp));
	arvore->raiz=NULL;
	arvore->hitCount = hitCount;
	arvore->promotionRate=promotionRate;
	arvore->epsilon=epsilon;
    arvore->prioridade_max=0;
	return arvore;
}

void setPrioridadeMax(SmuTreap t, int prioridade) {
    if (t == NULL) return;
    SmuTreapImp *impl = (SmuTreapImp*) t;
    if (prioridade >= 0) {
        impl->prioridade_max = prioridade;
    }
}



NodeSmu insertSmuT(SmuTreap t, double x, double y, Info i, DescritorTipoInfo d,
		FCalculaBoundingBox fCalcBb){
	if(t==NULL|| fCalcBb==NULL)return NULL;
	else{
		SmuTreapImp*treap = (SmuTreapImp*)t;
		treap->raiz = insertSmuTAux(t, treap->raiz, x, y, treap->epsilon, i, d, fCalcBb);
		return (NodeSmu)treap->raiz;
	}
}



NodeST* insertSmuTAux(SmuTreap t, NodeST *atual, double x, double y, double epsilon, Info i, DescritorTipoInfo d, 
	FCalculaBoundingBox fCalcBb){

	if(atual==NULL){
    

		NodeST*aux = (NodeST*)malloc(sizeof(NodeST));
		aux->bb = (boundingBox*)malloc(sizeof(boundingBox));
		if(aux == NULL )return NULL;
		if(aux->bb == NULL){
			free(aux);
			return NULL;
		}
        int prioridade = ((SmuTreapImp*)t)->prioridade_max;
		aux->x= x;
		aux->y = y;
		aux->prioridade= rand()%prioridade+1;
		aux->info=i;
		aux->descritor = d;
		aux->dir=aux->esq = NULL;
		double xa, ya, wa, ha;
		fCalcBb(d, i, &xa, &ya, &wa, &ha);
        /*printf("BB calculada: x=%.2f y=%.2f w=%.2f h=%.2f\n", xa, ya, wa, ha);*/
		aux->bb->x1 = xa;
		aux->bb->y1 = ya;
        aux->bb->x2 = xa + wa;
        aux->bb->y2 = ya + ha;
        aux->hits = 0;

        atualizaBoundingBox(aux);
        return aux;

    }


        if (fabs(x - atual->x) < epsilon && fabs(y - atual->y) < epsilon) {

        return atual;
    }
    if(x < atual->x - epsilon || (fabs(x - atual->x) < epsilon && y < atual->y - epsilon)){
    	atual->esq =insertSmuTAux(t, atual->esq, x, y, epsilon, i, d, fCalcBb);

    	if(atual->esq && atual->esq->prioridade>atual->prioridade)
    		return rotateRight(atual);
    	
    }else {
    	atual->dir = insertSmuTAux(t, atual->dir, x, y, epsilon, i, d, fCalcBb);

    	if(atual->dir&& atual->dir->prioridade>atual->prioridade)
    		return rotateLeft(atual);

	}

    atualizaBoundingBox(atual);
	return atual;
}
NodeST* rotateLeft(NodeST* node) {
    if (!node || !node->dir) return node; 
    
    NodeST* newRoot = node->dir;   
    node->dir = newRoot->esq;      
    newRoot->esq = node;

    atualizaBoundingBox(node);
    atualizaBoundingBox(newRoot);           
    
    return newRoot;  
}

NodeST* rotateRight(NodeST* node) {
    if (!node|| !node->esq) return node; 
    
    NodeST* newRoot = node->esq;   
    node->esq = newRoot->dir;      
    newRoot->dir = node;

    atualizaBoundingBox(node);
    atualizaBoundingBox(newRoot);           
    
    return newRoot;  
}

void atualizaBoundingBox(NodeST* node) {
    if (!node) return;

   
    double x1 = node->bb->x1;
    double y1 = node->bb->y1;
    double x2 = node->bb->x2;
    double y2 = node->bb->y2;

    
    if (node->esq) {
        if (node->esq->bb->x1 < x1) x1 = node->esq->bb->x1;
        if (node->esq->bb->y1 < y1) y1 = node->esq->bb->y1;
        if (node->esq->bb->x2 > x2) x2 = node->esq->bb->x2;
        if (node->esq->bb->y2 > y2) y2 = node->esq->bb->y2;
    }

    
    if (node->dir) {
        if (node->dir->bb->x1 < x1) x1 = node->dir->bb->x1;
        if (node->dir->bb->y1 < y1) y1 = node->dir->bb->y1;
        if (node->dir->bb->x2 > x2) x2 = node->dir->bb->x2;
        if (node->dir->bb->y2 > y2) y2 = node->dir->bb->y2;
    }

    
    node->bb->x1 = x1;
    node->bb->y1 = y1;
    node->bb->x2 = x2;
    node->bb->y2 = y2;
}


NodeSmu getNodeSmu(SmuTreap t, double x, double y){
    if(!t)printf("arvore durante o get vazia");
    SmuTreapImp* arvore = (SmuTreapImp*)t;
    return getNodeSmuAux(arvore->raiz, arvore,  x, y);
}



NodeSmu getNodeSmuAux(NodeST*raiz, SmuTreapImp*t, double x, double y){

    if(raiz==NULL)return NULL;

    if((x+t->epsilon)<raiz->bb->x1 || (x-t->epsilon)>raiz->bb->x2 || (y+t->epsilon)<raiz->bb->y1 
        || (y-t->epsilon)>raiz->bb->y2)
        return NULL;
    double distanciaPontos = sqrt (pow(raiz->x - x, 2) + pow(raiz->y -y, 2));

    if(distanciaPontos<= t->epsilon){
        raiz->hits +=1;
        if (raiz->hits == t->hitCount) {
        promoteNodeSmu((SmuTreap)t, (NodeSmu)raiz, t->promotionRate);
        
        }  

        return (NodeSmu)raiz;
    }

   if (x < raiz->x - t->epsilon || 
   (fabs(x - raiz->x) <= t->epsilon && y < raiz->y - t->epsilon)){
    return getNodeSmuAux(raiz->esq, t, x, y);

   }

    

   else if (x > raiz->x + t-> epsilon || 
        (fabs(x - raiz->x) <= t->epsilon && y > raiz->y + t->epsilon)){

     return getNodeSmuAux(raiz->dir, t,  x, y);
   }


    return NULL;
}

DescritorTipoInfo getTypeInfoSrbT(SmuTreap t, NodeSmu n) {
    SmuTreapImp* arvore = (SmuTreapImp*)t;
    NodeST* no = (NodeST*)n;
    no->hits +=1;
    if( no->hits == arvore->hitCount)
    promoteNodeSmu(t, n, arvore->promotionRate);
    return no->descritor;
}


void promoteNodeSmu(SmuTreap t, NodeSmu n, double promotionRate){
    SmuTreapImp*arvore = (SmuTreapImp*)t;
    NodeST *no = (NodeST*)n;
    no->prioridade *=promotionRate;
    arvore->raiz = promoteNodeSmuAux(arvore->raiz, no, arvore->epsilon);
}
// AGR TA CERTO PROFESSOR
NodeST* promoteNodeSmuAux(NodeST* raiz, NodeST* n, double epsilon) {
    if (raiz == NULL) {
        return NULL;
    }

    if (n->x < raiz->x - epsilon || (fabs(n->x - raiz->x) < epsilon && n->y < raiz->y - epsilon)) {
        raiz->esq = promoteNodeSmuAux(raiz->esq, n, epsilon);
        if (raiz->esq != NULL && raiz->esq->prioridade > raiz->prioridade) {
            return rotateRight(raiz);
        }
    } else if (n->x > raiz->x + epsilon || (fabs(n->x - raiz->x) < epsilon && n->y > raiz->y + epsilon)) {
        raiz->dir = promoteNodeSmuAux(raiz->dir, n, epsilon);
        if (raiz->dir != NULL && raiz->dir->prioridade > raiz->prioridade) {
            return rotateLeft(raiz);
        }
    }
    return raiz;
}

void removeNoSmuT(SmuTreap t, NodeSmu n){
    SmuTreapImp*arvore = (SmuTreapImp*)t;
    arvore->raiz = removeNoSmuTAux(arvore->raiz, (NodeST*)n, arvore->epsilon);
}

NodeST* removeNoSmuTAux(NodeST*raiz, NodeST*no, double epsilon){
    if(raiz==NULL)return NULL;

     if (fabs(raiz->x - no->x) < epsilon && fabs(raiz->y - no->y) < epsilon){
        if(raiz->esq ==NULL && raiz->dir==NULL){
            free(raiz->bb);
            free(raiz);
            return NULL;
        }else if(raiz->esq!=NULL && (raiz->dir ==NULL 
            || raiz->esq->prioridade>raiz->dir->prioridade)){
            raiz = rotateRight(raiz);
            raiz->dir=removeNoSmuTAux(raiz->dir, no, epsilon);
        }else{
            raiz = rotateLeft(raiz);
            raiz->esq=removeNoSmuTAux(raiz->esq, no, epsilon);
        }
    }
    else if(no->x < raiz->x - epsilon 
        || (fabs(no->x - raiz->x) < epsilon && no->y < raiz->y - epsilon))
        raiz->esq = removeNoSmuTAux (raiz->esq, no, epsilon);
    else raiz->dir = removeNoSmuTAux(raiz->dir, no, epsilon);

    atualizaBoundingBox(raiz);
    return raiz;

}


Info getInfoSmuT(SmuTreap t, NodeSmu n){
    SmuTreapImp* arvore = (SmuTreapImp*)t;
    NodeST* no = (NodeST*)n;
    no->hits +=1;
    if( no->hits == arvore->hitCount)
    promoteNodeSmu(t, n, arvore->promotionRate);
    return ((NodeST*)n)->info;
}

Info getBoundingBoxSmuT(SmuTreap t, NodeSmu n, double *x, double *y, double *w, double *h){
    if (n==NULL)return NULL;

    NodeST* no = (NodeST*)n;
    *x = no->bb->x1;
    *y = no->bb->y1;
    *w = no->bb->x2 - no->bb->x1;
    *h = no->bb->y2 - no->bb->y1;
}


bool getNodesDentroRegiaoSmuT(SmuTreap t, double x1, double y1, double x2, double y2, Lista L) {
    if (!t) return false;
    
    SmuTreapImp* arvore = (SmuTreapImp*)t;
    NodeST* raiz = arvore->raiz;

    bool encontrou = false;
    buscaNosDentro(t, raiz, x1, y1, x2, y2, L, &encontrou);
    return encontrou;
}

void buscaNosDentro(SmuTreap t, NodeST* no, double x1, double y1, double x2, double y2, Lista L, bool* encontrou) {
    if (!no) return;

    if (!intersecta(no->bb, x1, y1, x2, y2)) {
        return; 
    }

    if (no->x >= x1 && no->x <= x2 && no->y >= y1 && no->y <= y2) {
    double promotionRate = ((SmuTreapImp*)t)->promotionRate;
    //promoteNodeSmu(t, (NodeSmu)no, promotionRate);
        lista_insere(L, (Elemento) no);
        *encontrou = true;
    }

    buscaNosDentro(t, no->esq, x1, y1, x2, y2, L, encontrou);
    buscaNosDentro(t, no->dir, x1, y1, x2, y2, L, encontrou);
}

bool intersecta(boundingBox* bb, double x1, double y1, double x2, double y2) {
    return !(bb->x2 < x1 || bb->x1 > x2 || bb->y2 < y1 || bb->y1 > y2);
}

bool getInfosDentroRegiaoSmuT(SmuTreap t, double x1, double y1, double x2, double y2,
                              FdentroDeRegiao f, Lista L) {
    SmuTreapImp* arv = (SmuTreapImp*) t;
    return buscaInfosDentro(arv->raiz, t, x1, y1, x2, y2, f, L);

}
bool buscaInfosDentro(NodeST* no, SmuTreap t, double x1, double y1, double x2, double y2,
                      FdentroDeRegiao f, Lista L) {
    if (!no) return false;

    bool achou = false;

    if (intersecta(no->bb, x1, y1, x2, y2)) {

        if (f(t, no, no->info, x1, y1, x2, y2)) {
            double promotionRate = ((SmuTreapImp*)t)->promotionRate;
            SmuTreapImp* arvore = (SmuTreapImp*)t;
            no->hits++;            
            /*promoteNodeSmu(t, (NodeSmu)no, promotionRate);*/
            lista_insere(L, (Elemento)no);
            achou = true;
        }

   
        bool achou_esq = buscaInfosDentro( no->esq, t, x1, y1, x2, y2, f, L);
        bool achou_dir = buscaInfosDentro( no->dir, t, x1, y1, x2, y2, f, L);

        return achou || achou_esq || achou_dir;
    }

    
    return false;
}




bool getInfosAtingidoPontoSmuT(SmuTreap t, double x, double y, FpontoInternoAInfo f, Lista L) {
    if (t == NULL || L == NULL) return false;

    SmuTreapImp* treap = (SmuTreapImp*)t;


    bool encontrou = false;

    atingidosAux(treap, treap->raiz, x, y, f, L, &encontrou);

    return encontrou;
}

bool atingidosAux(SmuTreapImp* t, NodeST* no, double x, double y, FpontoInternoAInfo f, Lista L, bool* encontrou) {
    if (no == NULL) return false;


    if (x < no->bb->x1 || x > no->bb->x2 || y < no->bb->y1 || y > no->bb->y2) {
        return false;
    }

 
    if (f((SmuTreap)t, (NodeSmu)no, no->info, x, y)) {
        double promotionRate = ((SmuTreapImp*)t)->promotionRate;
       // promoteNodeSmu(t, no, promotionRate);
        lista_insere(L, (Elemento)no); 
        *encontrou = true;
    }


    atingidosAux(t, no->esq, x, y, f, L, encontrou);
    atingidosAux(t, no->dir, x, y, f, L, encontrou);

    return *encontrou;
}

void visitaProfundidadeAux(SmuTreap t, NodeST* no, FvisitaNo f, void* aux) {
    if (!no) return;

    f(t, (NodeSmu)no, no->info, no->x, no->y, aux);
    visitaProfundidadeAux(t, no->esq, f, aux);
    visitaProfundidadeAux(t, no->dir, f, aux);
}

void visitaProfundidadeSmuT(SmuTreap t, FvisitaNo f, void *aux) {
    if (!t || !f) return;

    NodeST* raiz = ((SmuTreapImp*)t)->raiz;
    visitaProfundidadeAux(t, raiz, f, aux);
}
void visitaLarguraSmuT(SmuTreap t, FvisitaNo f, void *aux) {
    if (!t || !f) return;

    Fila fila = fila_cria(10);  
    if (!fila) return;

    NodeST* raiz = ((SmuTreapImp*)t)->raiz;
    if (raiz == NULL) {
        fila_libera(fila);
        return;
    }

    fila_insere(fila, (Elemento)raiz);  

    while (!fila_vazia(fila)) {
        NodeST* noAtual = (NodeST*)fila_remove(fila);  

        f(t, (NodeSmu)noAtual, noAtual->info, noAtual->x, noAtual->y, aux);

        if (noAtual->esq) fila_insere(fila, (Elemento)noAtual->esq);
        if (noAtual->dir) fila_insere(fila, (Elemento)noAtual->dir);
    }

    fila_libera(fila);
}


 static NodeSmu procuraNoSmuTAux(NodeST* no, FsearchNo f, void* aux) {

    if (no == NULL) {
        return NULL;
    }

    if (f(NULL, (NodeSmu)no, no->info, 0, 0, aux)) {

        return (NodeSmu)no;
    }
    NodeSmu encontrado_esq = procuraNoSmuTAux(no->esq, f, aux);

    if (encontrado_esq != NULL) {
        return encontrado_esq;
    }
    return procuraNoSmuTAux(no->dir, f, aux);
}



NodeSmu procuraNoSmuT(SmuTreap t, FsearchNo f, void *aux) {
    if (!t || !f) {
        return NULL;
    }

    SmuTreapImp* arvore = (SmuTreapImp*)t;

    return procuraNoSmuTAux(arvore->raiz, f, aux);
}



double getEpsilonSmuT(SmuTreap t) {
    if (!t) return 0.0;
    SmuTreapImp* arv = (SmuTreapImp*) t;
    return arv->epsilon;
}

void printDotAux(FILE* fp, NodeST* no) {
    if (!no) return;

    // --- NOVA LÓGICA DE SELEÇÃO DE COR ---
    const char* cor_do_no;
    const char* nome_tipo;

    // Acessa o descritor do tipo da forma dentro do nó
    switch (no->descritor) {
        case 1:
            cor_do_no = "firebrick"; // Vermelho
            nome_tipo = "Circ";
            break;
        case 2:
            cor_do_no = "royalblue"; // Azul
            nome_tipo = "Ret";
            break;
        case 3:
            cor_do_no = "forestgreen"; // Verde
            nome_tipo = "Linha";
            break;
        case 4:
            cor_do_no = "purple"; // Roxo
            nome_tipo = "Txt";
            break;
        default:
            cor_do_no = "gray"; // Cor padrão
            nome_tipo = "???";
            break;
    }
    // --- FIM DA LÓGICA DE COR ---

    // Imprime a definição do nó com o novo label (sem ID) e a nova cor.
    fprintf(fp, "  \"%p\" [label=\"{Tipo: %s | Ancora: (%.1f, %.1f)} | {Hits: %d | Prio: %d}\", fillcolor=\"%s\"];\n",
            (void*)no,
            nome_tipo,
            no->x, no->y, 
            no->hits, no->prioridade,
            cor_do_no);

    // O resto da função continua igual
    if (no->esq) {
        fprintf(fp, "  \"%p\" -> \"%p\" [label=\"esq\"];\n", (void*)no, (void*)no->esq);
        printDotAux(fp, no->esq);
    }
    if (no->dir) {
        fprintf(fp, "  \"%p\" -> \"%p\" [label=\"dir\"];\n", (void*)no, (void*)no->dir);
        printDotAux(fp, no->dir);
    }
}

bool printDotSmuTreap(SmuTreap t, char *fn) {
    if (!t || !fn) return false;

    FILE *fp = fopen(fn, "w");
    if (!fp) return false;

    SmuTreapImp *arvore = (SmuTreapImp *)t;

    fprintf(fp, "digraph SmuTreap {\n");
    fprintf(fp, "  NodeSmu[shape=record, style=filled, fontcolor=white];\n");

    if (arvore->raiz) {
        printDotAux(fp, arvore->raiz);
    }

    fprintf(fp, "}\n");

    fclose(fp);
    return true;
}


void killNodes(NodeST *no) {
    if (!no) return;

    killNodes(no->esq);
    killNodes(no->dir);

    free(no->bb);
    free(no);
}

void killSmuTreap(SmuTreap t) {
    if (!t) return;

    SmuTreapImp *arvore = (SmuTreapImp *)t;

    killNodes(arvore->raiz);

    free(arvore);
}

int contaNosAux(NodeST* raiz) {
    if (raiz == NULL) return 0;
    return 1 + contaNos(raiz->esq) + contaNos(raiz->dir);
}

int contaNos(SmuTreap t) {
    SmuTreapImp* arvore = (SmuTreapImp*)t;
    return contaNosAux(arvore->raiz);
   
}

