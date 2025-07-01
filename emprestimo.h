#ifndef EMPRESTIMO_H
#define EMPRESTIMO_H

#include <stdio.h>
#include <time.h> // Usaremos esta biblioteca para registrar a data do empréstimo

/*
 * Define a estrutura de um registro de Empréstimo.
 * Cada registro representa um filme que está atualmente com um usuário.
 */
typedef struct Emprestimo {
    int codigo_filme;     // Chave para identificar o filme emprestado
    int codigo_usuario;   // Chave para identificar o usuário que pegou o filme
    time_t data_emprestimo; // Armazena a data e hora exatas do empréstimo
} TEmprestimo;

// --- Protótipos das Funções ---

// Retorna o tamanho em bytes de um registro de empréstimo.
int tamanho_registro_emprestimo();

// Aloca memória e cria uma nova struct TEmprestimo com os dados fornecidos.
TEmprestimo* emprestimo(int cod_filme, int cod_usuario, time_t data);

// Salva um registro de empréstimo no arquivo, na posição atual do cursor.
void salva_emprestimo(TEmprestimo* emp, FILE* out);

// Lê um registro de empréstimo do arquivo, da posição atual do cursor.
TEmprestimo* le_emprestimo(FILE* in);

#endif // EMPRESTIMO_H