#include "usuario.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int tamanho_registro_usuario() {
    return sizeof(int)      // cod
           + sizeof(char) * 100; // nome
}

TUser* usuario(int cod, char* nome) {
    TUser* user = (TUser*) malloc(sizeof(TUser));
    if (user) memset(user, 0, sizeof(TUser));
    user->codigo = cod;
    strcpy(user->nome, nome);
    return user;
}

void salva_usuario(TUser* user, FILE* out) {
    fwrite(&user->codigo, sizeof(int), 1, out);
    fwrite(user->nome, sizeof(char), sizeof(user->nome), out);
}

TUser* le_usuario(FILE* in) {
    TUser* user = (TUser*) malloc(sizeof(TUser));
    // Tenta ler o código. Se não conseguir (fim do arquivo), libera a memória e retorna NULL.
    if (fread(&user->codigo, sizeof(int), 1, in) <= 0) {
        free(user);
        return NULL;
    }
    fread(user->nome, sizeof(char), sizeof(user->nome), in);
    return user;
}

void imprime_usuario(TUser* user) {
    printf("Codigo: %d, Nome: %s\n", user->codigo, user->nome);
}

//-----------------------------------------------------------------------------------

void cadastrar_usuario() {
    char nome[100];
    printf("Digite o nome do usuario: ");
    scanf(" %[^\n]", nome);
    
    int cod = get_next_codigo("usuarios.dat", tamanho_registro_usuario());
    TUser* user = usuario(cod, nome);

    FILE* out = fopen("usuarios.dat", "ab");
    salva_usuario(user, out);
    fclose(out);
    
    printf("Usuario '%s' (cod: %d) cadastrado.\n", user->nome, user->codigo);
    free(user);
}

void listar_usuarios() {
    FILE* arq = fopen("usuarios.dat", "rb");
    if (!arq) { printf("Nenhum usuario cadastrado.\n"); return; }
    
    printf("\n--- LISTA DE USUARIOS ---\n");
    TUser* user;
    while ((user = le_usuario(arq)) != NULL) {
        imprime_usuario(user);
        free(user);
    }
    printf("-------------------------\n");
    fclose(arq);
}