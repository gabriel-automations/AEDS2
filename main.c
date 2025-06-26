#include <stdio.h>
#include <stdlib.h>
#include "filme.h"
#include "usuario.h"
#include "filme.c"
#include "usuario.c"



void exibe_menu() {
    printf("\n======= LOCADORA DE FILMES =======\n");
    printf("1. Cadastrar Usuario\n");
    printf("2. Cadastrar Filme\n");
    printf("3. Buscar Filme (Sequencial)\n");
    printf("4. Buscar Filme (Binaria)\n");
    printf("5. Ordenar Filmes por Ano\n");
    printf("6. Ordenar Filmes por C0digo\n");
    printf("7. Emprestar Filme\n");
    printf("8. Devolver Filme\n");
    printf("9. Listar Todos os Usuarios\n");
    printf("10. Listar Todos os Filmes\n");
    printf("11. Gerar Nova Base de Dados de Filmes\n");
    printf("0. Sair\n");
    printf("====================================================\n");
    printf("Escolha uma opcao: ");
}

int main() {
    int opcao;

    // Garante que o arquivo de filmes esteja ordenado por código para a busca binária
    FILE* arq_filmes = fopen("filmes.dat", "r+b");

    do {
        exibe_menu();
        scanf("%d", &opcao);
        

        switch (opcao) {
            case 1: cadastrar_usuario(); break;
            case 2: cadastrar_filme(); break;
            case 3: buscar_filme_sequencial(); break;
            case 4: buscar_filme_binaria(); break;
            case 5: ordena_filmes_por_ano(arq_filmes); break;
            case 6: ordena_filmes_por_codigo(arq_filmes); break;
            case 7: emprestar_filme(); break;
            case 8: devolver_filme(); break;
            case 9: listar_usuarios(); break;
            case 10: listar_filmes(); break;
            case 11: {
                int quantidade;
                printf("ATENCAO: Isso ira apagar a base de dados atual.\n");
                printf("Quantos filmes deseja gerar? ");
                scanf("%d", &quantidade);

                // Abre o arquivo em modo "wb" para apagar o conteúdo anterior e escrever do zero
                FILE* arq_out = fopen("filmes.dat", "wb");
                if (arq_out) {
                    criarBaseFilme(arq_out, quantidade);
                    fclose(arq_out);
                } else {
                    printf("Erro ao abrir o arquivo para escrita.\n");
                }
                break;
            }
            case 0: printf("Saindo...\n"); break;
            default: printf("Opcao invalida!\n"); break;
        }
    } while (opcao != 0);

    return 0;
}

// --- Implementação das Funções do Menu ---

int get_next_codigo(const char* filename, int record_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) return 1;
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        fclose(file);
        return 1;
    }
    fseek(file, -record_size, SEEK_END);
    int codigo;
    fread(&codigo, sizeof(int), 1, file);
    fclose(file);
    return codigo + 1;
}