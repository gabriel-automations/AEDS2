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

#endif // USUARIO_H