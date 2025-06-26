#include "filme.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> // Para a função trunc

//declaraçãod do protótipo da função abaixo
int get_next_codigo(const char *nome_arquivo, int tamanho_registro);

int tamanho_registro_filme() {
    return sizeof(int)      // codigo
           + sizeof(char) * 100 // titulo
           + sizeof(int)      // ano
           + sizeof(int)      // disponivel
           + sizeof(int);     // codigo_usuario_emprestado
}

TFilme* filme(int cod, char* titulo, int ano, int disponivel, int cod_usuario) {
    TFilme* filme = (TFilme*) malloc(sizeof(TFilme));
    if (filme) memset(filme, 0, sizeof(TFilme));
    filme->codigo = cod;
    strcpy(filme->titulo, titulo);
    filme->ano = ano;
    filme->disponivel = disponivel;
    filme->codigo_usuario_emprestado = cod_usuario;
    return filme;
}

void salva_filme(TFilme* filme, FILE* out) {
    fwrite(&filme->codigo, sizeof(int), 1, out);
    fwrite(filme->titulo, sizeof(char), sizeof(filme->titulo), out);
    fwrite(&filme->ano, sizeof(int), 1, out);
    fwrite(&filme->disponivel, sizeof(int), 1, out);
    fwrite(&filme->codigo_usuario_emprestado, sizeof(int), 1, out);
}

TFilme* le_filme(FILE* in) {
    TFilme* filme = (TFilme*) malloc(sizeof(TFilme));
    if (fread(&filme->codigo, sizeof(int), 1, in) <= 0) {
        free(filme);
        return NULL;
    }
    fread(filme->titulo, sizeof(char), sizeof(filme->titulo), in);
    fread(&filme->ano, sizeof(int), 1, in);
    fread(&filme->disponivel, sizeof(int), 1, in);
    fread(&filme->codigo_usuario_emprestado, sizeof(int), 1, in);
    return filme;
}

void imprime_filme(TFilme* filme) {
    printf("Cod: %d, Titulo: %s, Ano: %d, Disponivel: %s\n",
           filme->codigo, filme->titulo, filme->ano, filme->disponivel ? "Sim" : "Nao");
}

int tamanho_arquivo_filmes(FILE* arq) {
    fseek(arq, 0, SEEK_END);
    int tam = trunc(ftell(arq) / tamanho_registro_filme());
    return tam;
}

TFilme* busca_sequencial_filme(FILE* arq, int codigo, long* pos) {
    rewind(arq);
    TFilme* filme;
    while ((filme = le_filme(arq)) != NULL) {
        if (filme->codigo == codigo) {
            // Calcula a posição do início do registro encontrado
            *pos = ftell(arq) - tamanho_registro_filme();
            return filme;
        }
        free(filme); // Libera a memória se não for o filme procurado
    }
    *pos = -1;
    return NULL;
}

TFilme* busca_binaria_filme(FILE* arq, int codigo, long* pos) {
    int num_registros = tamanho_arquivo_filmes(arq);
    int inicio = 0, fim = num_registros - 1;
    TFilme* filme = NULL;

    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;
        *pos = meio * tamanho_registro_filme();
        fseek(arq, *pos, SEEK_SET);

        filme = le_filme(arq);
        if (!filme) break; // Segurança

        if (filme->codigo == codigo) {
            return filme;
        }
        if (filme->codigo < codigo) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
        free(filme); // Libera o registro do meio se não for o certo
    }
    *pos = -1;
    return NULL;
}

// Função genérica para ordenar em disco com Selection Sort
void selection_sort_disco(FILE* arq, int (*comparador)(TFilme*, TFilme*)) {
    int num_registros = tamanho_arquivo_filmes(arq);
    rewind(arq);

    for (int i = 0; i < num_registros - 1; i++) {
        // Encontra o registro mínimo no restante do arquivo
        long pos_min = i * tamanho_registro_filme();
        fseek(arq, pos_min, SEEK_SET);
        TFilme* f_min = le_filme(arq);

        for (int j = i + 1; j < num_registros; j++) {
            long pos_atual = j * tamanho_registro_filme();
            fseek(arq, pos_atual, SEEK_SET);
            TFilme* f_atual = le_filme(arq);

            if (comparador(f_atual, f_min) < 0) {
                free(f_min); // Libera o antigo mínimo
                f_min = f_atual;
                pos_min = pos_atual;
            } else {
                free(f_atual); // Libera o atual se não for o novo mínimo
            }
        }

        // Lê o registro da posição 'i' para a troca
        long pos_i = i * tamanho_registro_filme();
        fseek(arq, pos_i, SEEK_SET);
        TFilme* f_i = le_filme(arq);

        // Salva o mínimo na posição 'i'
        fseek(arq, pos_i, SEEK_SET);
        salva_filme(f_min, arq);

        // Salva o registro que estava na posição 'i' na antiga posição do mínimo
        fseek(arq, pos_min, SEEK_SET);
        salva_filme(f_i, arq);

        free(f_i);
        free(f_min);
    }
}

// Funções de comparação para o sort
int compara_por_ano(TFilme* a, TFilme* b) {
    return a->ano - b->ano;
}

int compara_por_codigo(TFilme* a, TFilme* b) {
    return a->codigo - b->codigo;
}

void ordena_filmes_por_ano(FILE* arq) {
    selection_sort_disco(arq, compara_por_ano);
}

void ordena_filmes_por_codigo(FILE* arq) {
    selection_sort_disco(arq, compara_por_codigo);
}

//----------------------------------------------------------

void cadastrar_filme() {
    char titulo[100];
    int ano;
    printf("Digite o titulo do filme: ");
    scanf(" %[^\n]", titulo);
    printf("Digite o ano de publicacao: ");
    scanf("%d", &ano);

    int cod = get_next_codigo("filmes.dat", tamanho_registro_filme());
    TFilme* f = filme(cod, titulo, ano, 1, -1);

    FILE* out = fopen("filmes.dat", "ab");
    salva_filme(f, out);
    fclose(out);

    printf("TFilme '%s' (cod: %d) cadastrado.\n", f->titulo, f->codigo);
    free(f);

}

void buscar_filme_sequencial() {
    int codigo;
    printf("Digite o codigo do filme para busca sequencial: ");
    scanf("%d", &codigo);

    FILE* arq = fopen("filmes.dat", "rb");
    if (!arq) { printf("Base de dados de filmes vazia.\n"); return; }
    
    long pos;
    TFilme* filme = busca_sequencial_filme(arq, codigo, &pos);
    
    if (filme) {
        printf("TFilme encontrado:\n");
        imprime_filme(filme);
        free(filme);
    } else {
        printf("TFilme com codigo %d nao encontrado.\n", codigo);
    }
    fclose(arq);
}

void buscar_filme_binaria() {
    int codigo;
    printf("Digite o codigo do filme para busca binaria: ");
    scanf("%d", &codigo);

    FILE* arq = fopen("filmes.dat", "rb");
    if (!arq) { printf("Base de dados de filmes vazia.\n"); return; }

    long pos;
    TFilme* filme = busca_binaria_filme(arq, codigo, &pos);
    
    if (filme) {
        printf("TFilme encontrado:\n");
        imprime_filme(filme);
        free(filme);
    } else {
        printf("TFilme com codigo %d nao encontrado.\n", codigo);
    }
    fclose(arq);
}

void ordenar_por_ano() {
    FILE* arq = fopen("filmes.dat", "r+b");
    if (!arq) { printf("Base de dados de filmes vazia.\n"); return; }

    printf("Ordenando filmes por ano...\n");
    ordena_filmes_por_ano(arq);
    printf("Ordenacao concluida.\n");
    
    rewind(arq);
    listar_filmes(); // Mostra o resultado

    // Reordena por código para a busca binária voltar a funcionar
    ordena_filmes_por_codigo(arq);
    fclose(arq);
}

void emprestar_filme() {
    int cod_filme, cod_usuario;
    printf("Digite o codigo do filme a ser emprestado: ");
    scanf("%d", &cod_filme);
    printf("Digite o codigo do usuario: ");
    scanf("%d", &cod_usuario);

    FILE* arq_filmes = fopen("filmes.dat", "r+b");
    if (!arq_filmes) { printf("Base de dados de filmes vazia.\n"); return; }

    long pos;
    TFilme* filme = busca_sequencial_filme(arq_filmes, cod_filme, &pos);
    
    if (!filme) {
        printf("ERRO: Filme nao encontrado.\n");
        fclose(arq_filmes);
        return;
    }
    if (!filme->disponivel) {
        printf("ERRO: TFilme '%s' ja esta emprestado.\n", filme->titulo);
        free(filme);
        fclose(arq_filmes);
        return;
    }

    // Modifica o filme
    filme->disponivel = 0;
    filme->codigo_usuario_emprestado = cod_usuario;

    // Sobrescreve no arquivo
    fseek(arq_filmes, pos, SEEK_SET);
    salva_filme(filme, arq_filmes);

    printf("TFilme '%s' emprestado com sucesso.\n", filme->titulo);
    free(filme);
    fclose(arq_filmes);
}

void devolver_filme() {
     int cod_filme;
    printf("Digite o codigo do filme a ser devolvido: ");
    scanf("%d", &cod_filme);

    FILE* arq_filmes = fopen("filmes.dat", "r+b");
    if (!arq_filmes) { printf("Base de dados de filmes vazia.\n"); return; }

    long pos;
    TFilme* filme = busca_sequencial_filme(arq_filmes, cod_filme, &pos);

    if (!filme) {
        printf("ERRO: TFilme nao encontrado.\n");
        fclose(arq_filmes);
        return;
    }
    if (filme->disponivel) {
        printf("ERRO: TFilme '%s' ja estava disponivel.\n", filme->titulo);
        free(filme);
        fclose(arq_filmes);
        return;
    }
    
    filme->disponivel = 1;
    filme->codigo_usuario_emprestado = -1;

    fseek(arq_filmes, pos, SEEK_SET);
    salva_filme(filme, arq_filmes);

    printf("TFilme '%s' devolvido com sucesso.\n", filme->titulo);
    free(filme);
    fclose(arq_filmes);
}

void listar_filmes() {
    FILE* arq = fopen("filmes.dat", "rb");
    if (!arq) { printf("Nenhum filme cadastrado.\n"); return; }

    printf("\n--- CATALOGO DE FILMES ---\n");
    TFilme* filme;
    while ((filme = le_filme(arq)) != NULL) {
        imprime_filme(filme);
        free(filme);
    }
    printf("---------------------------\n");
    fclose(arq);
}
