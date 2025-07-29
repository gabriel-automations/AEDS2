#ifndef USUARIO_H
#define USUARIO_H

#include <stdio.h>

typedef struct Usuario{
    int codigo;
    char nome[100];
} TUser;

// Retorna tamanho do registro Usuario em bytes
int tamanho_registro_usuario();

// Aloca e cria uma nova struct Usuario.
TUser* usuario(int cod, char* nome);

// Salva um Usuario no arquivo, na posição atual do cursor.
void salva_usuario(TUser* u, FILE* out);

// Lê um Usuario do arquivo, da posição atual do cursor. Retorna NULL se fim do arquivo.
TUser* le_usuario(FILE* in);

// Imprime os dados de um Usuario.
void imprime_usuario(TUser* u);

//Lista os usuarios
void listar_usuarios();

//Cadastra os usuarios
void cadastrar_usuario();

void criarBaseUsuario(FILE *out, int tam);

void embaralhar_vetor_usuario(int* vet, int tam);

void selection_sort_disco(FILE* arq, int (*comparador)(TUser*, TUser*));

int compara_por_codigo(TUser* a, TUser* b);

void ordena_usuarios_por_codigo(FILE* arq);

TUser* busca_sequencial_usuario(FILE* arq, int codigo, long* pos, int* contador_comparacoes);

void buscar_usuario_sequencial();

TUser* busca_binaria_usuario(FILE* arq, int codigo, long* pos, int* contador_comparacoes);

void buscar_usuario_binaria();

void ordenar_usuarios_selecao_natural();


#endif // USUARIO_H