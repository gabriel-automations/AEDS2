#ifndef FILME_H
#define FILME_H

#include <stdio.h>

typedef struct Filme{
    int codigo;
    char titulo[100];
    int ano;
    int disponivel;
    int codigo_usuario_emprestado;
} TFilme;

// Retorna tamanho do registro TFilme em bytes
int tamanho_registro_filme();

// Aloca e cria uma nova struct TFilme.
TFilme* film(int cod, char* titulo, int ano, int disponivel, int cod_usuario);

// Salva um TFilme no arquivo, na posição atual do cursor.
void salva_filme(TFilme* filme, FILE* out);

// Lê um TFilme do arquivo, da posição atual do cursor. Retorna NULL se fim do arquivo.
TFilme* le_filme(FILE* in);

// Imprime os dados de um TFilme.
void imprime_filme(TFilme* filme);

// Retorna a quantidade de registros no arquivo de filmes.
int tamanho_arquivo_filmes(FILE* arq);

// Busca um filme pelo código usando busca sequencial. Retorna o filme se encontrado, ou NULL.
// A posição em bytes do registro é armazenada no ponteiro 'pos'.
TFilme* busca_sequencial_filme(FILE* arq, int codigo, long* pos);

// Busca um filme pelo código usando busca binária. Retorna o filme se encontrado, ou NULL.
// A posição em bytes do registro é armazenada no ponteiro 'pos'.
// IMPORTANTE: O arquivo DEVE estar ordenado por código.
TFilme* busca_binaria_filme(FILE* arq, int codigo, long* pos);

// Ordena o arquivo de filmes em disco usando o campo 'ano' como chave.
// O método utilizado é o Selection Sort.
void ordena_filmes_por_ano(FILE* arq);

// Ordena o arquivo de filmes em disco usando o campo 'codigo' como chave.
void ordena_filmes_por_codigo(FILE* arq);

void cadastrar_filme();

void buscar_filme_sequencial();

void buscar_filme_binaria();

void ordenar_por_ano();

void emprestar_filme();

void devolver_filme();

void listar_filmes();

void criarBaseFilme(FILE* out, int tam);

void embaralhar_vetor_filme(int* vet, int tam);;

#endif // FILMEH