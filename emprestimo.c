#include "emprestimo.h"
#include <stdlib.h> // Para malloc e free
#include <string.h> // Para memset

/*
 * Retorna o tamanho em bytes de um registro de empréstimo.
 * Somamos os tamanhos de cada campo da struct TEmprestimo.
 * É essencial para usar fseek e navegar pelo arquivo de empréstimos.
 */
int tamanho_registro_emprestimo() {
    return sizeof(int)      // codigo_filme
           + sizeof(int)      // codigo_usuario
           + sizeof(time_t);  // data_emprestimo
}

/*
 * Função "construtora" que aloca memória e cria uma struct TEmprestimo.
 * Recebe os dados, aloca memória e preenche a struct.
 * Segue o mesmo padrão das suas funções 'filme()' e 'usuario()'.
 */
TEmprestimo* emprestimo(int cod_filme, int cod_usuario, time_t data) {
    TEmprestimo* emp = (TEmprestimo*) malloc(sizeof(TEmprestimo));
    // Limpa qualquer "lixo" de memória antes de preencher
    if (emp) memset(emp, 0, sizeof(TEmprestimo));
    emp->codigo_filme = cod_filme;
    emp->codigo_usuario = cod_usuario;
    emp->data_emprestimo = data;
    return emp;
}

/*
 * Salva os dados de uma struct TEmprestimo em um arquivo binário.
 * Usa fwrite para escrever os bytes da struct diretamente no arquivo.
 * Similar à sua função 'salva_filme()'.
 */
void salva_emprestimo(TEmprestimo* emp, FILE* out) {
    fwrite(&emp->codigo_filme, sizeof(int), 1, out);
    fwrite(&emp->codigo_usuario, sizeof(int), 1, out);
    fwrite(&emp->data_emprestimo, sizeof(time_t), 1, out);
}

/*
 * Lê os dados de um registro de empréstimo do arquivo e retorna uma struct.
 * Aloca memória para a struct e usa fread para preenchê-la.
 * Retorna NULL se chegar ao final do arquivo (EOF), um padrão importante
 * que você já usa em 'le_filme()' e 'le_usuario()'.
 */
TEmprestimo* le_emprestimo(FILE* in) {
    TEmprestimo* emp = (TEmprestimo*) malloc(sizeof(TEmprestimo));
    // Tenta ler o primeiro campo. Se o resultado de fread for 0 ou menos, significa que o arquivo acabou.
    if (fread(&emp->codigo_filme, sizeof(int), 1, in) <= 0) {
        free(emp); // Libera a memória alocada, já que não foi usada.
        return NULL;
    }
    // Se conseguiu ler o primeiro, lê o resto.
    fread(&emp->codigo_usuario, sizeof(int), 1, in);
    fread(&emp->data_emprestimo, sizeof(time_t), 1, in);
    return emp;
}