
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "geo.h"
#include "lista.h"


typedef struct {
    char cep[50];
    double x, y, w, h;
    char cfill[25]; 
    char cstrk[25]; 
    char sw[8];    
} Quadra;


Lista processaGeo(const char* caminho_geo) {
    FILE* arquivo = fopen(caminho_geo, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "ERRO: não foi possível abrir o arquivo .geo em '%s'\n", caminho_geo);
        return NULL;
    }

    Lista lista_de_quadras = lista_cria();
    if (lista_de_quadras == NULL) {
        fprintf(stderr, "ERRO: Falha ao criar a lista para as quadras.\n");
        fclose(arquivo);
        return NULL;
    }

    char cor_borda_atual[25] = "black";
    char cor_preenchimento_atual[25] = "lightgrey";
    char espessura_borda_atual[8] = "1";

    char buffer_linha[1024];

    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo) != NULL) {
        
        char* ptr = buffer_linha;
 
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        if (ptr[0] == '\0' || ptr[0] == '\n' || ptr[0] == '\r' || ptr[0] == '#') {
            continue;
        }

        char comando[10];
        sscanf(ptr, "%s", comando);

        if (strcmp(comando, "cq") == 0) {
            sscanf(ptr, "cq %s %s %s",
                   espessura_borda_atual, cor_preenchimento_atual, cor_borda_atual);

        } else if (strcmp(comando, "q") == 0) {
            char cep[50];
            double x, y, w, h;
            int itens_lidos = sscanf(ptr, "q %s %lf %lf %lf %lf", cep, &x, &y, &w, &h);

            if (itens_lidos == 5) {
                Quadra* nova_quadra = malloc(sizeof(Quadra));
                if (nova_quadra == NULL) {
                    fprintf(stderr, "ERRO: Falha de alocação de memória para nova quadra.\n");
                    continue;
                }

                strcpy(nova_quadra->cep, cep);
                nova_quadra->x = x;
                nova_quadra->y = y;
                nova_quadra->w = w;
                nova_quadra->h = h;
                strcpy(nova_quadra->cfill, cor_preenchimento_atual);
                strcpy(nova_quadra->cstrk, cor_borda_atual);
                strcpy(nova_quadra->sw, espessura_borda_atual);
                
                lista_insere(lista_de_quadras, nova_quadra);
            }
        }
    }

    fclose(arquivo);
    return lista_de_quadras;
}