#include <stdio.h>
#include <stdlib.h>

#include "filme.h"
#include "usuario.h"
#include "emprestimo.h"

#include "filme.c"
#include "usuario.c"
#include "emprestimo.c"

void consultar_emprestimo_filme();
void contar_filmes_por_usuario();

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
    printf("15. Consultar emprestimo por filme\n");
    printf("16. Contar filmes por usuario\n");
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

/*
 * Função para registrar uma mensagem em um arquivo de log.
 * Ela adiciona a data e a hora atuais a cada mensagem.
 */
void registra_log(const char* mensagem) {
    // Abre o arquivo "locadora.log" em modo "a" (append - adicionar ao final)
    FILE* log_file = fopen("locadora.log", "a");
    if (log_file == NULL) {
        // Se não for possível abrir o arquivo de log, simplesmente retorna.
        // Não queremos que o programa principal pare por causa de um log.
        return;
    }

    // Pega a data e hora atuais do sistema
    time_t agora = time(NULL);
    // Converte para um formato de texto legível
    char* data_formatada = ctime(&agora);
    // Remove a quebra de linha '\n' que ctime adiciona no final
    data_formatada[strcspn(data_formatada, "\n")] = 0;

    // Escreve a data/hora e a mensagem no arquivo
    fprintf(log_file, "[%s] %s\n", data_formatada, mensagem);
    
    // Fecha o arquivo
    fclose(log_file);
}

int main() {
    int opcao;

    
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
            case 15: consultar_emprestimo_filme(); break;
            case 16: contar_filmes_por_usuario(); break;
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

void consultar_emprestimo_filme() {
    int cod_filme;
    printf("Digite o codigo do filme para consultar: ");
    scanf("%d", &cod_filme);

    FILE* arq_emp = fopen("emprestimos.dat", "rb");
    if (!arq_emp) {
        printf("INFO: Nenhum emprestimo registrado.\n");
        return;
    }

    TEmprestimo* emp;
    int encontrado = 0;
    while ((emp = le_emprestimo(arq_emp)) != NULL) {
        if (emp->codigo_filme == cod_filme) {
            // Encontramos o empréstimo do filme, agora precisamos buscar o nome do usuário.
            FILE* arq_usuarios = fopen("usuarios.dat", "rb");
            if (arq_usuarios) {
                long pos;
                TUser* usuario = busca_sequencial_usuario(arq_usuarios, emp->codigo_usuario, &pos);
                if (usuario) {
                    printf("INFO: O filme de codigo %d esta emprestado para o usuario: %s (Cod: %d).\n",
                           cod_filme, usuario->nome, usuario->codigo);
                    free(usuario);
                } else {
                    printf("INFO: O filme de codigo %d esta emprestado para o usuario de codigo %d (nome nao encontrado).\n",
                           cod_filme, emp->codigo_usuario);
                }
                fclose(arq_usuarios);
            }
            encontrado = 1;
            break; // Para o loop, pois cada filme só pode ser emprestado uma vez.
        }
        free(emp);
    }

    if (!encontrado) {
        printf("INFO: O filme de codigo %d nao consta como emprestado.\n", cod_filme);
    }

    fclose(arq_emp);
}

void contar_filmes_por_usuario() {
    int cod_usuario;
    printf("Digite o codigo do usuario para consultar: ");
    scanf("%d", &cod_usuario);

    // Primeiro, vamos verificar se o usuário realmente existe para dar um feedback melhor.
    FILE* arq_usuarios = fopen("usuarios.dat", "rb");
    if (!arq_usuarios) {
        printf("ERRO: Base de dados de usuarios vazia.\n");
        return;
    }
    long pos;
    TUser* usuario = busca_sequencial_usuario(arq_usuarios, cod_usuario, &pos);
    fclose(arq_usuarios);
    if (!usuario) {
        printf("ERRO: Usuario com codigo %d nao encontrado.\n", cod_usuario);
        return;
    }
    // O nome do usuário será útil para a mensagem final.
    char nome_usuario[100];
    strcpy(nome_usuario, usuario->nome);
    free(usuario);


    // Agora, vamos contar os empréstimos para esse usuário.
    FILE* arq_emp = fopen("emprestimos.dat", "rb");
    if (!arq_emp) {
        printf("INFO: Nenhum emprestimo registrado no sistema.\n");
        return;
    }

    TEmprestimo* emp;
    int contador = 0;
    while ((emp = le_emprestimo(arq_emp)) != NULL) {
        if (emp->codigo_usuario == cod_usuario) {
            contador++;
        }
        free(emp);
    }

    printf("INFO: O usuario %s (Cod: %d) possui %d filme(s) emprestado(s).\n",
           nome_usuario, cod_usuario, contador);

    fclose(arq_emp);
}