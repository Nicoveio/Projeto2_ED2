#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"      
#include "geo.h"       
#include "via.h"       
#include "svg.h"        
#include "graph.h"     
#include "lista.h"     
#include "qry.h" 

int main(int argc, char *argv[]) {
 
    //  1: LER E VALIDAR OS PARÂMETROS DE ENTRADA

    printf("1. Processando parâmetros da linha de comando...\n");
    void* params = criarParametros();
    if (!processarArgumentos(params, argc, argv)) {
        destruirParametros(params);
        return 1;
    }

    // Obter os caminhos dos arquivos necessários
    char* caminho_geo = getCaminhoCompletoGeo(params);
    char* caminho_via = getCaminhoCompletoVia(params);
    char* caminho_qry = NULL;
    char* caminho_svg_saida = NULL;
    char* caminho_txt_saida = NULL;
    char* caminho_svg_base = NULL;

    // Validação essencial: precisamos dos dois arquivos para o mapa base.
    if (caminho_geo == NULL || caminho_via == NULL) {
        fprintf(stderr, "Erro: Os parâmetros -f (arq.geo) e -v (arq.via) são obrigatórios para gerar o mapa.\n");
        free(caminho_geo);
        free(caminho_via);
        destruirParametros(params);
        return 1;
    }
    printf("   -> Arquivo GEO: %s\n", caminho_geo);
    printf("   -> Arquivo VIA: %s\n", caminho_via);


    //  2: CARREGAR A CIDADE PARA A MEMÓRIA

    printf("\n2. Carregando dados da cidade...\n");
    
    // a) Carrega as quadras do arquivo .geo
    printf("   - Lendo quadras do arquivo .geo...\n");
    Lista lista_de_quadras = processaGeo(caminho_geo);
    if (lista_de_quadras == NULL) {
        fprintf(stderr, "   [FALHOU] Erro fatal ao carregar as quadras.\n");
        free(caminho_geo);
        free(caminho_via);
        destruirParametros(params);
        return 1;
    }

    // b) Carrega a malha viária do arquivo .via
    printf("   - Lendo malha viária do arquivo .via...\n");
    Graph g = carregarGrafoDeArquivoVia(caminho_via);
    if (g == NULL) {
        fprintf(stderr, "   [FALHOU] Erro fatal ao carregar o grafo.\n");
        // TODO: Liberar a lista_de_quadras aqui
        free(caminho_geo);
        free(caminho_via);
        destruirParametros(params);
        return 1;
    }
    printf("   [OK] Dados da cidade carregados com sucesso! (%d vértices)\n", getTotalNodes(g));


       if (temArquivoQry(params)) {
        printf("\n3. Processando arquivo de consultas...\n");
        caminho_qry = getCaminhoCompletoQry(params);
        caminho_svg_saida = getCaminhoSvgConsulta(params);
        caminho_txt_saida = getCaminhoTxtConsulta(params);

        ResultadosConsulta resultados = processaQry(g, lista_de_quadras, caminho_qry, caminho_txt_saida, calculaCustoAresta);
        
        if (resultados) { 
            gerarSvgFinal(g, lista_de_quadras, resultados, caminho_svg_saida);
            liberaResultadosConsulta(resultados);
        }
    }
    printf("\n4. Finalizando e liberando memória...\n");

    if (lista_de_quadras) {
        Iterador it = lista_iterador(lista_de_quadras);
        while(iterador_tem_proximo(it)) {
            free(iterador_proximo(it));
        }
        iterador_destroi(it);
        lista_libera(lista_de_quadras);
    }
    
    
    free(caminho_geo);
    free(caminho_via);
    free(caminho_qry);
    free(caminho_svg_saida);
    free(caminho_txt_saida);
    killDG(g);
    
    printf("   [OK] Memória liberada.\n\n");
    printf("Execução concluída com sucesso!\n");

    return 0;
}