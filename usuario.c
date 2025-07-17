#include "usuario.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


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

int tamanho_arquivo_usuarios(FILE* arq) {
    fseek(arq, 0, SEEK_END);
    int tam = trunc(ftell(arq) / tamanho_registro_usuario());
    return tam;
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

//__________________________________________________________________________________________________________

void criarBaseUsuario(FILE *out, int tam)
{
    int vet[tam];
    TUser *u;

    for (int i = 0; i < tam; i++)
    {
        vet[i] = i + 1;
    }
    
    embaralhar_vetor_usuario(vet, tam);

    printf("\nGerando a base de dados...\n");

    for (int i = 0; i < tam; i++)
    {
        u = usuario(vet[i],"Usuario da silva");
        salva_usuario(u, out);
    }
    free(u);
}

void embaralhar_vetor_usuario(int* vet, int tam) {
    int tmp;
    // Semeia o gerador de números aleatórios para garantir sequências diferentes a cada execução
    srand(time(NULL)); 
    
    // Calcula o número de trocas (60% do tamanho do vetor)
    int trocas = (tam * 60) / 100;

    for (int t = 0; t < trocas; t++) {
        // Escolhe dois índices aleatórios no intervalo [0, tam-1]
        int i = rand() % tam;
        int j = rand() % tam;
        
        // Troca os valores nos índices i e j
        tmp = vet[i];
        vet[i] = vet[j];
        vet[j]=tmp;
    }
}

// Função genérica para ordenar em disco com Selection Sort
void selection_sort_disco_usuario(FILE* arq, int (*comparador)(TUser*, TUser*), int* contador_comparacoes) {
    int num_registros = tamanho_arquivo_usuarios(arq);
    rewind(arq);

    for (int i = 0; i < num_registros - 1; i++) {
        // Encontra o registro mínimo no restante do arquivo
        long pos_min = i * tamanho_registro_usuario();
        fseek(arq, pos_min, SEEK_SET);
        TUser* u_min = le_usuario(arq);

        for (int j = i + 1; j < num_registros; j++) {
            long pos_atual = j * tamanho_registro_usuario();
            fseek(arq, pos_atual, SEEK_SET);
           TUser* u_atual = le_usuario(arq);

            (*contador_comparacoes)++; // <-- ADICIONE ESTA LINHA
            if (comparador(u_atual, u_min) < 0) {
                free(u_min);
                u_min = u_atual;
                pos_min = pos_atual;
            } else {
                free(u_atual);
}
        }

        // Lê o registro da posição 'i' para a troca
        long pos_i = i * tamanho_registro_usuario();
        fseek(arq, pos_i, SEEK_SET);
        TUser* u_i = le_usuario(arq);

        // Salva o mínimo na posição 'i'
        fseek(arq, pos_i, SEEK_SET);
        salva_usuario(u_min, arq);

        // Salva o registro que estava na posição 'i' na antiga posição do mínimo
        fseek(arq, pos_min, SEEK_SET);
        salva_usuario(u_i, arq);

        free(u_i);
        free(u_min);
    }
}

int compara_por_codigo_u(TUser* a, TUser* b) {
    return a->codigo - b->codigo;
}

// Em usuario.c
void ordena_usuarios_por_codigo(FILE* arq) {
    printf("Ordenando usuarios por codigo. Isso pode levar um tempo...\n");

    // Variáveis para medição
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a medição de tempo
    inicio = clock();

    // Chama a nova versão da função de ordenação
    selection_sort_disco_usuario(arq, compara_por_codigo_u, &comparacoes);

    // Termina a medição de tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    printf("Ordenacao concluida!\n");

    // Prepara e registra a mensagem de log
    sprintf(log_mensagem, "Ordenacao de USUARIOS por codigo (Selection Sort em Disco): Concluida. Tempo: %.6f segundos. Comparacoes: %d.",
            tempo_decorrido, comparacoes);
    registra_log(log_mensagem);
}

TUser* busca_sequencial_usuario(FILE* arq, int codigo, long* pos, int* contador_comparacoes) {
    rewind(arq);
    TUser* usuario;
    while ((usuario = le_usuario(arq)) != NULL) {
        (*contador_comparacoes)++; // Incrementa a cada usuário lido e comparado

        if (usuario->codigo == codigo) {
            *pos = ftell(arq) - tamanho_registro_usuario();
            return usuario;
        }
        free(usuario);
    }
    *pos = -1;
    return NULL;
}

// Em usuario.c
void buscar_usuario_sequencial() {
    int codigo;
    printf("Digite o codigo do Usuario para busca sequencial: ");
    scanf("%d", &codigo);

    FILE* arq = fopen("usuarios.dat", "rb");
    if (!arq) { printf("Base de dados de usuarios vazia.\n"); return; }

    // Variáveis para medição de desempenho
    long pos;
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a contagem do tempo
    inicio = clock();

    // Chama a nova versão da função, passando o endereço do contador
    TUser* usuario = busca_sequencial_usuario(arq, codigo, &pos, &comparacoes);

    // Termina a contagem do tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Prepara a mensagem de log
    if (usuario) {
        printf("Usuario encontrado:\n");
        imprime_usuario(usuario);
        sprintf(log_mensagem, "Busca sequencial pelo usuario de codigo %d: SUCESSO. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
        free(usuario);
    } else {
        printf("Usuario com codigo %d nao encontrado.\n", codigo);
        sprintf(log_mensagem, "Busca sequencial pelo usuario de codigo %d: FALHA. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
    }

    // Registra o log e fecha o arquivo
    registra_log(log_mensagem);
    fclose(arq);
}

TUser* busca_binaria_usuario(FILE* arq, int codigo, long* pos, int* contador_comparacoes) {
    int num_registros = tamanho_arquivo_usuarios(arq);
    int inicio = 0, fim = num_registros - 1;
    TUser* usuario = NULL;

    while (inicio <= fim) {
        (*contador_comparacoes)++; // Incrementa para cada iteração do loop
        int meio = inicio + (fim - inicio) / 2;
        *pos = meio * tamanho_registro_usuario();
        fseek(arq, *pos, SEEK_SET);

        usuario = le_usuario(arq);
        if (!usuario) break;

        (*contador_comparacoes)++; // Incrementa para a comparação de igualdade
        if (usuario->codigo == codigo) {
            return usuario;
        }
        (*contador_comparacoes)++; // Incrementa para a comparação de menor/maior
        if (usuario->codigo < codigo) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
        free(usuario);
    }
    *pos = -1;
    return NULL;
}

void buscar_usuario_binaria() {
    int codigo;
    printf("Digite o codigo do usuario para busca binaria: ");
    scanf("%d", &codigo);

    FILE* arq = fopen("usuarios.dat", "rb");
    if (!arq) { printf("Base de dados de usuario vazia.\n"); return; }

    // Variáveis para medição de desempenho
    long pos;
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a contagem do tempo
    inicio = clock();

    // Chama a nova versão da função, passando o endereço do contador
    TUser* usuario = busca_binaria_usuario(arq, codigo, &pos, &comparacoes);

    // Termina a contagem do tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Prepara a mensagem de log
    if (usuario) {
        printf("Usuario encontrado:\n");
        imprime_usuario(usuario);
        sprintf(log_mensagem, "Busca binaria pelo usuario de codigo %d: SUCESSO. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
        free(usuario);
    } else {
        printf("Usuario com codigo %d nao encontrado.\n", codigo);
        sprintf(log_mensagem, "Busca binaria pelo usuario de codigo %d: FALHA. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
    }

    // Registra o log e fecha o arquivo
    registra_log(log_mensagem);
    fclose(arq);
}

