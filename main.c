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
    printf("5. Buscar Usuario (Sequencial)\n");
    printf("6. Buscar Usuario (Binaria)\n");
    printf("7. Ordenar Filmes por Ano\n");
    printf("8. Ordenar Filmes por Codigo\n");
    printf("9. Ordenar Usuarios por Codigo\n");
    printf("10. Emprestar Filme\n");
    printf("11. Devolver Filme\n");
    printf("12. Listar Todos os Usuarios\n");
    printf("13. Listar Todos os Filmes\n");
    printf("14. Gerar Nova Base de Dados de Filmes e Usuarios\n");
    printf("0. Sair\n");
    printf("====================================================\n");
    printf("Escolha uma opcao: ");
}

void geradorBases(){
    int quantidadeFilme;
    int quantidadeUsuarios;

    printf("ATENCAO: Isso ira apagar a base de dados atual.\n");
    printf("Quantos filmes deseja gerar? ");
    scanf("%d", &quantidadeFilme);
    printf("Quantos usuarios deseja gerar? ");
    scanf("%d", &quantidadeUsuarios);

    // Abre o arquivo em modo "wb" para apagar o conteúdo anterior e escrever do zero
    FILE* arq_outF = fopen("filmes.dat", "wb");
    if (arq_outF) {
        criarBaseFilme(arq_outF, quantidadeFilme);
        fclose(arq_outF);
    } else {
        printf("Erro ao abrir o arquivo para escrita.\n");
    }
    FILE* arq_outU = fopen("usuarios.dat", "wb");
    if (arq_outU) {
        criarBaseUsuario(arq_outU, quantidadeUsuarios);
        fclose(arq_outU);
    } else {
    printf("Erro ao abrir o arquivo para escrita.\n");
    }
}

int main() {
    int opcao;

    // Garante que o arquivo de filmes esteja ordenado por código para a busca binária
    FILE* arq_filmes = fopen("filmes.dat", "r+b");
    FILE* arq_usuarios = fopen("usuarios.dat", "r+b");

    do {
        exibe_menu();
        scanf("%d", &opcao);
        

        switch (opcao) {
            case 1: cadastrar_usuario(); break;
            case 2: cadastrar_filme(); break;
            case 3: buscar_filme_sequencial(); break;
            case 4: buscar_filme_binaria(); break;
            case 5: buscar_usuario_sequencial(); break;
            case 6: buscar_usuario_binaria(); break;
            case 7: ordena_filmes_por_ano(arq_filmes); break;
            case 8: ordena_filmes_por_codigo(arq_filmes); break;
            case 9: ordena_usuarios_por_codigo(arq_usuarios); break;
            case 10: emprestar_filme(); break;
            case 11: devolver_filme(); break;
            case 12: listar_usuarios(); break;
            case 13: listar_filmes(); break;
            case 14: geradorBases(); break;
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