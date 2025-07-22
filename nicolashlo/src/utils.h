#ifndef UTILS_H
#define UTILS_H

// ========== FUNÇÕES DE CRIAÇÃO/DESTRUIÇÃO ==========

/**
 * Cria um novo objeto de parâmetros
 * @return Ponteiro para o objeto criado ou NULL em caso de erro
 */
void* criarParametros();

/**
 * Processa os argumentos da linha de comando
 * @param parametros Objeto de parâmetros criado
 * @param argc Número de argumentos
 * @param argv Array de argumentos
 * @return 1 se sucesso, 0 se erro
 */
int processarArgumentos(void *parametros, int argc, char *argv[]);

/**
 * Libera a memória do objeto de parâmetros
 * @param parametros Objeto a ser liberado
 */
void destruirParametros(void *parametros);

// ========== FUNÇÕES GET - PARÂMETROS BÁSICOS ==========

/**
 * Obtém o diretório de entrada
 * @param parametros Objeto de parâmetros
 * @return String com o diretório ou NULL
 */
const char* getDiretorioEntrada(void *parametros);

/**
 * Obtém o nome do arquivo .geo
 * @param parametros Objeto de parâmetros
 * @return String com o nome do arquivo ou NULL
 */
const char* getArquivoGeo(void *parametros);

/**
 * Obtém o diretório de saída
 * @param parametros Objeto de parâmetros
 * @return String com o diretório ou NULL
 */
const char* getDiretorioSaida(void *parametros);

/**
 * Obtém o nome do arquivo .qry
 * @param parametros Objeto de parâmetros
 * @return String com o nome do arquivo ou NULL se não especificado
 */
const char* getArquivoQry(void *parametros);

/**
 * Verifica se foi especificado arquivo de consultas
 * @param parametros Objeto de parâmetros
 * @return 1 se tem arquivo .qry, 0 caso contrário
 */
int temArquivoQry(void *parametros);

// ========== FUNÇÕES GET - CAMINHOS COMPLETOS ==========

/**
 * Obtém o caminho completo do arquivo .geo
 * @param parametros Objeto de parâmetros
 * @return String com caminho completo (deve ser liberada com free) ou NULL
 */
char* getCaminhoCompletoGeo(void *parametros);

/**
 * Obtém o caminho completo do arquivo .qry
 * @param parametros Objeto de parâmetros
 * @return String com caminho completo (deve ser liberada com free) ou NULL
 */
char* getCaminhoCompletoQry(void *parametros);

// ========== FUNÇÕES GET - NOMES BASE ==========

/**
 * Obtém o nome base do arquivo .geo (sem extensão)
 * @param parametros Objeto de parâmetros
 * @return String com nome base (deve ser liberada com free) ou NULL
 */
char* getNomeBaseGeo(void *parametros);

/**
 * Obtém o nome base do arquivo .qry (sem extensão)
 * @param parametros Objeto de parâmetros
 * @return String com nome base (deve ser liberada com free) ou NULL
 */
char* getNomeBaseQry(void *parametros);

// ========== FUNÇÕES GET - ARQUIVOS DE SAÍDA ==========

/**
 * Gera o caminho completo do arquivo SVG base
 * @param parametros Objeto de parâmetros
 * @return String com caminho (deve ser liberada com free) ou NULL
 */
char* getCaminhoSvgBase(void *parametros);

/**
 * Gera o caminho completo do arquivo SVG de consulta
 * @param parametros Objeto de parâmetros
 * @return String com caminho (deve ser liberada com free) ou NULL se não há .qry
 */
char* getCaminhoSvgConsulta(void *parametros);

/**
 * Gera o caminho completo do arquivo TXT de consulta
 * @param parametros Objeto de parâmetros
 * @return String com caminho (deve ser liberada com free) ou NULL se não há .qry
 */
char* getCaminhoTxtConsulta(void *parametros);

// ========== FUNÇÃO DE DEBUG ==========

/**
 * Imprime os parâmetros processados (para debug)
 * @param parametros Objeto de parâmetros
 */
void imprimirParametros(void *parametros);

/**
 * @brief Obtém o valor máximo de prioridade para os nós da árvore.
 * Este valor é usado para gerar prioridades aleatórias na inserção de nós.
 * @param parametros O ponteiro opaco para a struct de parâmetros.
 * @return O valor da prioridade máxima definido pelo usuário (via -p) ou o valor padrão.
 */
int getPrioridadeMax(void* parametros);

/**
 * @brief Obtém a contagem de 'hits' (acessos) necessária para promover um nó.
 * @param parametros O ponteiro opaco para a struct de parâmetros.
 * @return O número de acessos definido pelo usuário (via -hc) ou o valor padrão.
 */
int getHitCount(void* parametros);

/**
 * @brief Obtém a taxa (fator multiplicativo) de promoção para os nós da árvore.
 * @param parametros O ponteiro opaco para a struct de parâmetros.
 * @return O fator de promoção definido pelo usuário (via -pr) ou o valor padrão.
 */
double getPromotionRate(void* parametros);

// Em utils.h, junto com as outras funções getCaminho...

/**
 * Gera o caminho completo do arquivo .dot base (pós-geo)
 * @param parametros Objeto de parâmetros
 * @return String com caminho (deve ser liberada com free) ou NULL
 */
char* getCaminhoDotBase(void *parametros);

/**
 * Gera o caminho completo do arquivo .dot de consulta (pós-qry)
 * @param parametros Objeto de parâmetros
 * @return String com caminho (deve ser liberada com free) ou NULL
 */
char* getCaminhoDotConsulta(void *parametros);


/**
 * Cria uma duplicata de uma string na memória heap.
 * É uma implementação portável da função não-padrão strdup, usando
 * apenas funções da biblioteca padrão do C.
 * @param s String a ser copiada.
 * @return Ponteiro para a nova string alocada, ou NULL em caso de erro.
 * O chamador é responsável por liberar a memória retornada com free().
 */
char* duplicar_string(const char* s);

/**
 * Obtém o nome do arquivo .via
 * @param parametros Objeto de parâmetros
 * @return String com o nome do arquivo ou NULL se não especificado
 */
const char* getArquivoVia(void *parametros);

/**
 * Obtém o caminho completo do arquivo .via
 * @param parametros Objeto de parâmetros
 * @return String com caminho completo (deve ser liberada com free) ou NULL
 */
char* getCaminhoCompletoVia(void *parametros);


#endif 

