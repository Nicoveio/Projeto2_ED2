// Arquivo: qry.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "qry.h"
#include "graph.h"
#include "lista.h"
#include "hash.h"
#include "utils.h"
#include "svg.h"
#include "geo.h" 

typedef struct {
    char cep[50];
    double x, y, w, h;
    char cfill[25], cstrk[25], sw[8];
} Quadra;

typedef struct {
    double x;
    double y;
    bool definido; 
} Coordenadas;

typedef struct {
    char nome[50];
    Lista caminho_curto;
    Lista caminho_rapido;
} Percurso;



void processaQry(Graph g, Lista quadras, const char* caminho_qry, const char* caminho_svg_saida, const char* caminho_txt_saida) {
    FILE* arquivo_qry = fopen(caminho_qry, "r");
    FILE* arquivo_txt = fopen(caminho_txt_saida, "w");
    if (!arquivo_qry || !arquivo_txt) {
        fprintf(stderr, "ERRO: Não foi possível abrir os arquivos de consulta ou de saída de texto.\n");
        if (arquivo_qry) fclose(arquivo_qry);
        if (arquivo_txt) fclose(arquivo_txt);
        return;
    }


    Coordenadas registradores[11];
    for (int i = 0; i < 11; i++) registradores[i].definido = false;

    hashTable tabela_de_percursos = createHashTable(29);
    Lista elementos_para_desenhar = lista_cria();
    
    char buffer_linha[1024];
    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo_qry) != NULL) {
        buffer_linha[strcspn(buffer_linha, "\r\n")] = '\0';
        if (buffer_linha[0] == '\0' || buffer_linha[0] == '#') continue;

        //GUIA QUE MONTEI COM QRY DO QUE FAZER EM CADA FUNÇÃO

        fprintf(arquivo_txt, "Comando: %s\n", buffer_linha);

        // COMANDO @o? 
        if (strncmp(buffer_linha, "@o?", 3) == 0) {
            char reg_str[4], cep[50], face_str[2];
            double num;
            sscanf(buffer_linha, "@o? %s %s %s %lf", reg_str, cep, face_str, &num);
            
            Quadra* q_alvo = encontrarQuadraPorCep(quadras, cep);
            if (q_alvo) {
                Coordenadas coord = calcularCoordenada(q_alvo, face_str[0], num);
                int indice_reg = atoi(reg_str + 1);
                if (indice_reg >= 0 && indice_reg <= 10) {
                    registradores[indice_reg] = coord;
                    registradores[indice_reg].definido = true;
                    fprintf(arquivo_txt, "  -> Coordenada (%.2f, %.2f) armazenada em %s\n", coord.x, coord.y, reg_str);
                    //  Adicionar o marcador @o? à lista de 'elementos_para_desenhar'
                }
            } else {
                fprintf(arquivo_txt, "  -> ERRO: CEP '%s' não encontrado.\n", cep);
            }
        
        //  COMANDO alag
        } else if (strncmp(buffer_linha, "alag", 4) == 0) {

            //'Parsear' n, x, y, w, h da linha.
            //Usar a SmuTreap (getNodeSmusDentroRegiaoSmuT?) para encontrar os vértices na área.
            //Para cada vértice encontrado, percorrer sua lista de adjacência e chamar disableEdge.
            //Guardar as arestas desabilitadas associadas ao 'n'.
            //Adicionar o retângulo de alagamento à 'elementos_para_desenhar'.
            //Escrever as arestas desabilitadas no .txt.

        // COMANDO dren 
        } else if (strncmp(buffer_linha, "dren", 4) == 0) {
            //Parsear 'n' da linha.
            //Recuperar a lista de arestas associadas ao 'n' do alagamento.
            //Chamar enableEdge para cada uma.
            //Escrever as arestas reabilitadas no .txt.

        //  COMANDO sg 
        } else if (strncmp(buffer_linha, "sg", 2) == 0) {
            //Parsear nome, x, y, w, h da linha.
            //Usar a SmuTreap para encontrar os vértices na área?
            //Chamar createSubgraphDG do graph.c.
            //Adicionar o retângulo pontilhado à 'elementos_para_desenhar'.

        // COMANDO p? 
        } else if (strncmp(buffer_linha, "p?", 2) == 0) {
            //parsear np, nome_subgrafo, reg1, reg2.
            //Pegar as coordenadas dos registradores.
            //Chamar encontrarNoMaisProximo para obter os IDs de início e fim.
            // Chamar findPath duas vezes (distância e tempo).
            // Criar uma struct Percurso, guardar as duas listas de caminho nela.
            // Armazenar o ponteiro para a struct Percurso na 'tabela_de_percursos' com a chave 'np'.
            // Escrever a descrição textual do caminho no .txt.

        // COMANDO join 
        } else if (strncmp(buffer_linha, "join", 4) == 0) {
            //1. Semelhante a p?

        // COMANDO shw 
        } else if (strncmp(buffer_linha, "shw", 3) == 0) {
            // Parsear np, cmc, cmr.
            // Buscar o Percurso na 'tabela_de_percursos' pela chave 'np'.
            // Adicionar o percurso e suas cores à lista 'elementos_para_desenhar'.
        }

        fprintf(arquivo_txt, "\n");
    }

    printf("Processamento de consultas concluído. Gerando SVG final...\n");

    hashTableDestroy(tabela_de_percursos);
    lista_libera(elementos_para_desenhar);

    fclose(arquivo_qry);
    fclose(arquivo_txt);
}