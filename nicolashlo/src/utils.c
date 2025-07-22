#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura interna do TAD (encapsulada)
typedef struct {
    char *dir_entrada;      // -e (opcional, padrão = ".")
    char *arquivo_geo;      // -f (obrigatório)
    char *dir_saida;        // -o (obrigatório)
    char *arquivo_qry;      // -q (opcional)
    char *arquivo_via;
    int prioridade_max;     // -p (opcional)
    int hit_count;          // -hc (opcional)
    double promotion_rate;  // -pr (opcional);
} Parametros;

static int strcasecmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        int c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

// ========== FUNÇÕES AUXILIARES ESTÁTICAS ==========


// Função para verificar se arquivo existe
static int arquivo_existe(const char *caminho) {
    if (!caminho) return 0;
    
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo) {
        fclose(arquivo);
        return 1;
    }
    return 0;
}

// Função para verificar extensão do arquivo
static int verificar_extensao(const char *arquivo, const char *extensao_esperada) {
    if (!arquivo || !extensao_esperada) return 0;
    
    size_t len_arquivo = strlen(arquivo);
    size_t len_extensao = strlen(extensao_esperada);
    
    // Arquivo deve ser maior que a extensão
    if (len_arquivo <= len_extensao) return 0;
    
    // Verificar se termina com a extensão esperada (case insensitive)
    const char *extensao_arquivo = arquivo + len_arquivo - len_extensao;
    return (strcasecmp(extensao_arquivo, extensao_esperada) == 0);
}


// Função para criar caminho completo
static char* criar_caminho_completo(const char *dir, const char *arquivo) {
    if (!dir || !arquivo) return NULL;
    
    size_t len_dir = strlen(dir);
    size_t len_arq = strlen(arquivo);
    size_t len_total = len_dir + len_arq + 2; // +2 para '/' e '\0'
    
    char *caminho = malloc(len_total);
    if (!caminho) return NULL;
    
    // Verificar se dir já termina com '/'
    if (len_dir > 0 && dir[len_dir-1] == '/') {
        snprintf(caminho, len_total, "%s%s", dir, arquivo);
    } else {
        snprintf(caminho, len_total, "%s/%s", dir, arquivo);
    }
    
    return caminho;
}

// Função para extrair nome base do arquivo (sem extensão e sem caminho)
static char* extrair_nome_base(const char *arquivo) {
    if (!arquivo) return NULL;
    
    // Encontrar último '/' ou '\' (para Windows)
    const char *nome = arquivo;
    const char *p = arquivo;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            nome = p + 1;
        }
        p++;
    }
    
    // Copiar nome sem extensão
    const char *ponto = strrchr(nome, '.');
    size_t len = ponto ? (size_t)(ponto - nome) : strlen(nome);
    
    char *resultado = malloc(len + 1);
    if (resultado) {
        strncpy(resultado, nome, len);
        resultado[len] = '\0';
    }
    
    return resultado;
}

// Função para mostrar mensagem de uso
static void mostrar_uso(const char *nome_programa) {
    fprintf(stderr, "Uso: %s [-e path] -f arq.geo [-q consulta.qry] -o dir [-p num] [-hc num] [-pr decimal]\n", nome_programa);
    fprintf(stderr, "\nParâmetros:\n");
    fprintf(stderr, "  -e path        Diretório de entrada (opcional, padrão: diretório atual)\n");
    fprintf(stderr, "  -f arq.geo     Arquivo .geo (obrigatório)\n");
    fprintf(stderr, "  -q arq.qry     Arquivo .qry (opcional)\n");
    fprintf(stderr, "  -o dir         Diretório de saída (obrigatório)\n");
    fprintf(stderr, "  -p num         Prioridade máxima (opcional, padrão: 10000)\n");
    fprintf(stderr, "  -hc num        Hit count (opcional, padrão: 3)\n");
    fprintf(stderr, "  -pr decimal    Promotion rate (opcional, padrão: 1.10)\n");
}

// ========== FUNÇÕES DE PROCESSAMENTO E VALIDAÇÃO ==========

// Função para processar argumentos
static int processar_argumentos_interno(int argc, char *argv[], Parametros *params) {
    // Inicializar estrutura com valores padrão
    memset(params, 0, sizeof(Parametros));
    params->prioridade_max = 10000;
    params->hit_count = 3;
    params->promotion_rate = 1.10;
    
    // Verificar se há argumentos suficientes
    if (argc < 5) { // Mínimo: programa -f arquivo.geo -o diretorio
        fprintf(stderr, "Erro: Argumentos insuficientes\n");
        mostrar_uso(argv[0]);
        return 0;
    }
    
    // Processar argumentos manualmente
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -e requer um argumento\n");
                return 0;
            }
            params->dir_entrada = malloc(strlen(argv[i + 1]) + 1);
            if (!params->dir_entrada) {
                fprintf(stderr, "Erro: Falha na alocação de memória\n");
                return 0;
            }
            strcpy(params->dir_entrada, argv[i + 1]);
            i++; // pular próximo argumento
        }
        else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -f requer um argumento\n");
                return 0;
            }
            
            // VERIFICAR EXTENSÃO .geo
            if (!verificar_extensao(argv[i + 1], ".geo")) {
                fprintf(stderr, "Erro: Arquivo deve ter extensão .geo\n");
                fprintf(stderr, "Arquivo fornecido: %s\n", argv[i + 1]);
                return 0;
            }
            
            params->arquivo_geo = malloc(strlen(argv[i + 1]) + 1);
            if (!params->arquivo_geo) {
                fprintf(stderr, "Erro: Falha na alocação de memória\n");
                return 0;
            }
            strcpy(params->arquivo_geo, argv[i + 1]);
            i++; // pular próximo argumento
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -o requer um argumento\n");
                return 0;
            }
            params->dir_saida = malloc(strlen(argv[i + 1]) + 1);
            if (!params->dir_saida) {
                fprintf(stderr, "Erro: Falha na alocação de memória\n");
                return 0;
            }
            strcpy(params->dir_saida, argv[i + 1]);
            i++; // pular próximo argumento
        }
        else if (strcmp(argv[i], "-q") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -q requer um argumento\n");
                return 0;
            }
        
            
            // VERIFICAR EXTENSÃO .qry
            if (!verificar_extensao(argv[i + 1], ".qry")) {
                fprintf(stderr, "Erro: Arquivo de consulta deve ter extensão .qry\n");
                fprintf(stderr, "Arquivo fornecido: %s\n", argv[i + 1]);
                return 0;
            }
            
            params->arquivo_qry = malloc(strlen(argv[i + 1]) + 1);
            if (!params->arquivo_qry) {
                fprintf(stderr, "Erro: Falha na alocação de memória\n");
                return 0;
            }
            strcpy(params->arquivo_qry, argv[i + 1]);
            i++; // pular próximo argumento
        }
        else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -p requer um número inteiro\n");
                return 0;
            }
            int valor = atoi(argv[i + 1]);
            if (valor <= 0) {
                fprintf(stderr, "Erro: -p deve ser um número inteiro positivo\n");
                return 0;
            }
            params->prioridade_max = valor;
            i++;
        }
        else if (strcmp(argv[i], "-hc") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -hc requer um número inteiro\n");
                return 0;
            }
            int valor = atoi(argv[i + 1]);
            if (valor <= 0) {
                fprintf(stderr, "Erro: -hc deve ser um número inteiro positivo\n");
                return 0;
            }
            params->hit_count = valor;
            i++;
        }
        else if (strcmp(argv[i], "-pr") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Erro: -pr requer um número decimal\n");
                return 0;
            }
            double valor = atof(argv[i + 1]);
            if (valor <= 0.0) {
                fprintf(stderr, "Erro: -pr deve ser um número decimal positivo\n");
                return 0;
            }
            params->promotion_rate = valor;
            i++;
        }
        else if (strcmp(argv[i], "-v") == 0) {
             if (i + 1 >= argc) {
                  fprintf(stderr, "Erro: -v requer um argumento\n");
                  return 0;
             }
             params->arquivo_via = duplicar_string (argv[i + 1]);
             if (!params->arquivo_via) {
                  fprintf(stderr, "Erro: Falha na alocação de memória\n");
                  return 0;
              }
             i++; // pular próximo argumento
        }
        else {
            fprintf(stderr, "Erro: Argumento desconhecido '%s'\n", argv[i]);
            mostrar_uso(argv[0]);
            return 0;
        }
    }
    
    return 1;
}

// Função para validar parâmetros
static int validar_parametros_interno(Parametros *params) {
    // Verificar parâmetros obrigatórios
    if (!params->arquivo_geo) {
        fprintf(stderr, "Erro: Parâmetro -f (arquivo .geo) é obrigatório\n");
        return 0;
    }
    
    if (!params->dir_saida) {
        fprintf(stderr, "Erro: Parâmetro -o (diretório de saída) é obrigatório\n");
        return 0;
    }
    
    // Definir diretório de entrada padrão se não especificado
    if (!params->dir_entrada) {
        params->dir_entrada = malloc(2);
        if (!params->dir_entrada) {
            fprintf(stderr, "Erro: Falha na alocação de memória\n");
            return 0;
        }
        strcpy(params->dir_entrada, ".");
    }
    
    // Verificar se arquivo .geo existe
    char *caminho_geo = criar_caminho_completo(params->dir_entrada, params->arquivo_geo);
    if (!caminho_geo) {
        fprintf(stderr, "Erro: Falha ao criar caminho do arquivo .geo\n");
        return 0;
    }
    
    if (!arquivo_existe(caminho_geo)) {
        fprintf(stderr, "Erro: Arquivo .geo '%s' não encontrado\n", caminho_geo);
        free(caminho_geo);
        return 0;
    }
    free(caminho_geo);
    
    // Verificar arquivo .qry se especificado
    if (params->arquivo_qry) {
        char *caminho_qry = criar_caminho_completo(params->dir_entrada, params->arquivo_qry);
        if (!caminho_qry) {
            fprintf(stderr, "Erro: Falha ao criar caminho do arquivo .qry\n");
            return 0;
        }
        
        if (!arquivo_existe(caminho_qry)) {
            fprintf(stderr, "Erro: Arquivo de consultas .qry '%s' não encontrado\n", caminho_qry);
            free(caminho_qry);
            return 0;
        }
        free(caminho_qry);
    }
    
    // Testar se diretório de saída é gravável
    char *teste_saida = criar_caminho_completo(params->dir_saida, "test.tmp");
    if (!teste_saida) {
        fprintf(stderr, "Erro: Falha ao criar caminho de teste\n");
        return 0;
    }
    
    FILE *fp = fopen(teste_saida, "w");
    if (!fp) {
        fprintf(stderr, "Erro: Diretório de saída '%s' não existe ou não é gravável\n", params->dir_saida);
        free(teste_saida);
        return 0;
    }
    fclose(fp);
    remove(teste_saida);
    free(teste_saida);
    
    return 1;
}

// ========== FUNÇÕES PÚBLICAS DO TAD ==========
char* duplicar_string(const char* s){
    size_t tamanho = strlen(s) + 1;
    char* nova_string = malloc(tamanho);
    if (nova_string == NULL) {
        return NULL;
    }
    strcpy(nova_string, s);
    return nova_string;
}
// Criar novo objeto Parametros
void* criarParametros() {
    Parametros *params = malloc(sizeof(Parametros));
    if (params) {
        memset(params, 0, sizeof(Parametros));
    }
    return params;
}

// Processar argumentos da linha de comando
int processarArgumentos(void *parametros, int argc, char *argv[]) {
    if (!parametros) return 0;
    
    Parametros *params = (Parametros*)parametros;
    
    if (!processar_argumentos_interno(argc, argv, params)) {
        return 0;
    }
    
    if (!validar_parametros_interno(params)) {
        return 0;
    }
    
    return 1;
}

// Liberar memória do objeto
void destruirParametros(void *parametros) {
    if (!parametros) return;
    
    Parametros *params = (Parametros*)parametros;
    
    free(params->dir_entrada);
    free(params->arquivo_geo);
    free(params->dir_saida);
    free(params->arquivo_qry);
    free(params);
}

// ========== FUNÇÕES GET ==========

const char* getDiretorioEntrada(void *parametros) {
    if (!parametros) return NULL;
    return ((Parametros*)parametros)->dir_entrada;
}

const char* getArquivoGeo(void *parametros) {
    if (!parametros) return NULL;
    return ((Parametros*)parametros)->arquivo_geo;
}

const char* getDiretorioSaida(void *parametros) {
    if (!parametros) return NULL;
    return ((Parametros*)parametros)->dir_saida;
}

const char* getArquivoQry(void *parametros) {
    if (!parametros) return NULL;
    return ((Parametros*)parametros)->arquivo_qry;
}

int temArquivoQry(void *parametros) {
    if (!parametros) return 0;
    return (((Parametros*)parametros)->arquivo_qry != NULL);
}

int getPrioridadeMax(void* parametros) {
    if (!parametros) return 10000;
    return ((Parametros*)parametros)->prioridade_max;
}

int getHitCount(void* parametros) {
    if (!parametros) return 3;
    return ((Parametros*)parametros)->hit_count;
}

double getPromotionRate(void* parametros) {
    if (!parametros) return 1.10;
    return ((Parametros*)parametros)->promotion_rate;
}

// ========== FUNÇÕES DE CAMINHOS ==========

char* getCaminhoCompletoGeo(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    return criar_caminho_completo(params->dir_entrada, params->arquivo_geo);
}

char* getCaminhoCompletoQry(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    if (!params->arquivo_qry) return NULL;
    return criar_caminho_completo(params->dir_entrada, params->arquivo_qry);
}

char* getNomeBaseGeo(void *parametros) {
    if (!parametros) return NULL;
    return extrair_nome_base(((Parametros*)parametros)->arquivo_geo);
}

char* getNomeBaseQry(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    if (!params->arquivo_qry) return NULL;
    return extrair_nome_base(params->arquivo_qry);
}

char* getCaminhoSvgBase(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    
    char *nome_base = extrair_nome_base(params->arquivo_geo);
    if (!nome_base) return NULL;
    
    size_t len = strlen(params->dir_saida) + strlen(nome_base) + 10;
    char *caminho = malloc(len);
    if (caminho) {
        snprintf(caminho, len, "%s/%s.svg", params->dir_saida, nome_base);
    }
    
    free(nome_base);
    return caminho;
}

char* getCaminhoSvgConsulta(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    if (!params->arquivo_qry) return NULL;
    
    char *nome_base_geo = extrair_nome_base(params->arquivo_geo);
    char *nome_base_qry = extrair_nome_base(params->arquivo_qry);
    
    if (!nome_base_geo || !nome_base_qry) {
        free(nome_base_geo);
        free(nome_base_qry);
        return NULL;
    }
    
    size_t len = strlen(params->dir_saida) + strlen(nome_base_geo) + strlen(nome_base_qry) + 15;
    char *caminho = malloc(len);
    if (caminho) {
        snprintf(caminho, len, "%s/%s-%s.svg", params->dir_saida, nome_base_geo, nome_base_qry);
    }
    
    free(nome_base_geo);
    free(nome_base_qry);
    return caminho;
}

char* getCaminhoTxtConsulta(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    if (!params->arquivo_qry) return NULL;
    
    char *nome_base_geo = extrair_nome_base(params->arquivo_geo);
    char *nome_base_qry = extrair_nome_base(params->arquivo_qry);
    
    if (!nome_base_geo || !nome_base_qry) {
        free(nome_base_geo);
        free(nome_base_qry);
        return NULL;
    }
    
    size_t len = strlen(params->dir_saida) + strlen(nome_base_geo) + strlen(nome_base_qry) + 15;
    char *caminho = malloc(len);
    if (caminho) {
        snprintf(caminho, len, "%s/%s-%s.txt", params->dir_saida, nome_base_geo, nome_base_qry);
    }
    
    free(nome_base_geo);
    free(nome_base_qry);
    return caminho;
}

// ========== FUNÇÃO DE DEBUG ==========

void imprimirParametros(void *parametros) {
    if (!parametros) {
        printf("Parâmetros: NULL\n");
        return;
    }
    
    Parametros *params = (Parametros*)parametros;
    printf("=== PARÂMETROS PROCESSADOS ===\n");
    printf("Diretório entrada: %s\n", params->dir_entrada ? params->dir_entrada : "(não definido)");
    printf("Arquivo .geo: %s\n", params->arquivo_geo ? params->arquivo_geo : "(não definido)");
    printf("Diretório saída: %s\n", params->dir_saida ? params->dir_saida : "(não definido)");
    printf("Arquivo .qry: %s\n", params->arquivo_qry ? params->arquivo_qry : "(nenhum)");
    printf("Prioridade máxima: %d\n", params->prioridade_max);
    printf("Hit count: %d\n", params->hit_count);
    printf("Promotion rate: %.2f\n", params->promotion_rate);
    printf("=============================\n");
}


char* getCaminhoDotBase(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    
    char *nome_base = extrair_nome_base(params->arquivo_geo);
    if (!nome_base) return NULL;
    
    size_t len = strlen(params->dir_saida) + strlen(nome_base) + 10;
    char *caminho = malloc(len);
    if (caminho) {
        snprintf(caminho, len, "%s/%s.dot", params->dir_saida, nome_base);
    }
    
    free(nome_base);
    return caminho;
}

char* getCaminhoDotConsulta(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    if (!params->arquivo_qry) return NULL;
    
    char *nome_base_geo = extrair_nome_base(params->arquivo_geo);
    char *nome_base_qry = extrair_nome_base(params->arquivo_qry);
    
    if (!nome_base_geo || !nome_base_qry) { 
        return NULL; }
    
    // Aloca espaço para "caminho/nome_geo-nome_qry.dot\0"
    size_t len = strlen(params->dir_saida) + strlen(nome_base_geo) + strlen(nome_base_qry) + 15;
    char *caminho = malloc(len);
    if (caminho) {
        snprintf(caminho, len, "%s/%s-%s.dot", params->dir_saida, nome_base_geo, nome_base_qry);
    }
    
    free(nome_base_geo);
    free(nome_base_qry);
    return caminho;
}
const char* getArquivoVia(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    return params->arquivo_via;
}

char* getCaminhoCompletoVia(void *parametros) {
    if (!parametros) return NULL;
    Parametros *params = (Parametros*)parametros;
    if (!params->arquivo_via) return NULL;
    return criar_caminho_completo(params->dir_entrada, params->arquivo_via);
}

