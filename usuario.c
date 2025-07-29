#include "usuario.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <math.h>


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

// ------------------------------------------
// |               TRABALHO 2               |
// ------------------------------------------


#include <stdbool.h> // Necessário para usar o tipo 'bool'
// Define o tamanho da memória interna (quantos registros podemos manter em memória)
#define MEMORIA_INTERNA 6


/*
 * Função auxiliar para encontrar o usuário com o menor código em um vetor de usuários.
 * Retorna o índice do menor usuário. Ignora posições nulas ou congeladas.
 */
int encontrar_menor_usuario(TUser* memoria[], bool congelado[], int tam) {
    int idx_menor = -1;
    TUser* menor_usuario = NULL;

    for (int i = 0; i < tam; i++) {
        // Se o registro não estiver congelado e existir
        if (!congelado[i] && memoria[i] != NULL) {
            if (menor_usuario == NULL || memoria[i]->codigo < menor_usuario->codigo) {
                menor_usuario = memoria[i];
                idx_menor = i;
            }
        }
    }
    return idx_menor;
}

/*
 * ETAPA 1: SELEÇÃO NATURAL
 * Lê o arquivo de usuários e cria várias partições (arquivos) menores já ordenadas.
 */
int selecao_natural_usuarios(FILE* in) {
    // Inicializa o reservatório (memória interna)
    TUser* memoria[MEMORIA_INTERNA];
    bool congelado[MEMORIA_INTERNA];
    int registros_na_memoria = 0; // Nome da variável alterado para maior clareza

    // Preenche a memória inicial
    for (int i = 0; i < MEMORIA_INTERNA; i++) {
        memoria[i] = le_usuario(in);
        congelado[i] = false;
        if (memoria[i] != NULL) {
            registros_na_memoria++; // Conta apenas os registros que foram realmente lidos
        }
    }

    int num_particoes = 0;
    // O loop principal agora depende diretamente dos registros que estão na memória
    while (registros_na_memoria > 0) {
        // Abre um novo arquivo de partição para escrita
        char nome_particao[30];
        sprintf(nome_particao, "particao_usuario_%d.dat", num_particoes);
        FILE* out_particao = fopen(nome_particao, "wb");
        if (!out_particao) {
            printf("Erro ao criar arquivo de particao.\n");
            return -1;
        }

        // Descongela todos os registros para iniciar uma nova partição
        for(int i = 0; i < MEMORIA_INTERNA; i++) {
            congelado[i] = false;
        }
        
        int menor_idx;

        // Loop para gerar uma partição
        while ((menor_idx = encontrar_menor_usuario(memoria, congelado, MEMORIA_INTERNA)) != -1) {
            // Pega o ponteiro para o menor usuário para salvá-lo
            TUser* menor_atual = memoria[menor_idx];
            
            // Salva o menor no arquivo de partição
            salva_usuario(menor_atual, out_particao);

            // Tenta substituir o registro salvo por um novo do arquivo principal
            memoria[menor_idx] = le_usuario(in);

            // Se a substituição falhou (fim do arquivo de entrada)
            if (memoria[menor_idx] == NULL) {
                registros_na_memoria--;
            } else {
                // Se o novo registro for menor que o último escrito, ele é congelado
                if (menor_atual->codigo > memoria[menor_idx]->codigo) {
                    congelado[menor_idx] = true;
                }
            }
            free(menor_atual); 
        }

        fclose(out_particao);
        num_particoes++;
    }

    return num_particoes;
}

#define FATOR_INTERCALACAO 500 // Quantas partições vamos intercalar por vez

// Função auxiliar para a intercalação de um lote de arquivos
int intercalar_lote(char* nomes_entrada[], int qtd_arquivos, char* nome_saida, int* comparacoes) {
    FILE* out = fopen(nome_saida, "wb");
    if (!out) {
        printf("Erro ao criar arquivo de saida temporario: %s\n", nome_saida);
        return -1;
    }

    FILE* entradas[qtd_arquivos];
    TUser* competidores[qtd_arquivos];

    for (int i = 0; i < qtd_arquivos; i++) {
        entradas[i] = fopen(nomes_entrada[i], "rb");
        if (entradas[i]) {
            competidores[i] = le_usuario(entradas[i]);
        } else {
            printf("ERRO CRITICO: Nao foi possivel abrir a particao de entrada: %s\n", nomes_entrada[i]);
            competidores[i] = NULL;
        }
    }

    while (true) {
        int idx_menor = -1;
        TUser* menor_usuario = NULL;
        for (int i = 0; i < qtd_arquivos; i++) {
            if (competidores[i] != NULL) {
                if (menor_usuario == NULL || competidores[i]->codigo < menor_usuario->codigo) {
                    menor_usuario = competidores[i];
                    idx_menor = i;
                }
                if(menor_usuario != competidores[i]) (*comparacoes)++;
            }
        }
        if (idx_menor == -1) break;
        
        salva_usuario(menor_usuario, out);
        free(menor_usuario);
        competidores[idx_menor] = le_usuario(entradas[idx_menor]);
    }

    fclose(out);

    for (int i = 0; i < qtd_arquivos; i++) {
        if(entradas[i]) {
            fclose(entradas[i]);
        }

        int tentativas = 0;
        while (remove(nomes_entrada[i]) != 0 && tentativas < 5) {
            tentativas++;
            #ifdef _WIN32
                Sleep(10);
            #else
                usleep(10000);
            #endif
        }
        if (tentativas >= 5) {
            perror("ERRO CRITICO: Nao foi possivel apagar o arquivo apos varias tentativas");
            printf(" -> Arquivo problematico: %s\n", nomes_entrada[i]);
        }
        
        free(nomes_entrada[i]);
    }
    
    return 0;
}


/*
 * ETAPA 2: INTERCALAÇÃO ÓTIMA
 * Junta as partições ordenadas em um único arquivo final ordenado.
 */
int intercalacao_otima_usuarios(int num_particoes) {
    if (num_particoes == 0) return 0;

    // Se só existe uma partição, ela já está ordenada. Apenas renomeamos.
    if (num_particoes == 1) {
        char nome_particao[30];
        sprintf(nome_particao, "particao_usuario_0.dat");
        remove("usuarios.dat");
        rename(nome_particao, "usuarios.dat");
        return 0;
    }

    int comparacoes = 0;
    // Aloca memória para guardar os nomes de todas as partições atuais
    char** nomes_particoes = malloc(sizeof(char*) * num_particoes);
    for (int i = 0; i < num_particoes; i++) {
        nomes_particoes[i] = malloc(sizeof(char) * 30);
        sprintf(nomes_particoes[i], "particao_usuario_%d.dat", i);
    }

    int num_passo = 0;
    // Continua intercalando enquanto houver mais de 1 partição
    while (num_particoes > 1) {
        int qtd_lotes = ceil((double)num_particoes / FATOR_INTERCALACAO); //numero de lotes que serao usados
        char** nomes_saida_lotes = malloc(sizeof(char*) * qtd_lotes); // memoria para o nome de saida dos lotes

        // Processa lote por lote
        for (int i = 0; i < qtd_lotes; i++) {
            int inicio_lote = i * FATOR_INTERCALACAO;
            int fim_lote = inicio_lote + FATOR_INTERCALACAO;
            //reduz o fim_lote se ultrapassar o número total de partições
            if (fim_lote > num_particoes) {
                fim_lote = num_particoes;
            }
            int qtd_arquivos_lote = fim_lote - inicio_lote; // Quantidade de arquivos no lote atual

            // Define o nome do arquivo de saída para este lote
            nomes_saida_lotes[i] = malloc(sizeof(char) * 30);
            sprintf(nomes_saida_lotes[i], "temp_passo%d_lote%d.dat", num_passo, i);
            
            // Chama a função para intercalar o lote atual
            intercalar_lote(&nomes_particoes[inicio_lote], qtd_arquivos_lote, nomes_saida_lotes[i], &comparacoes);
        }
        
        free(nomes_particoes); // Libera a lista de nomes antiga
        nomes_particoes = nomes_saida_lotes; // A nova lista de partições são as saídas dos lotes
        num_particoes = qtd_lotes; // Atualiza o número de partições para o próximo passo
        num_passo++;
    }

    // Ao final, sobrará apenas um arquivo na lista, que é o arquivo final ordenado
    remove("usuarios.dat");
    rename(nomes_particoes[0], "usuarios.dat");
    
    // Limpa a memória do último nome de arquivo restante
    free(nomes_particoes[0]);
    free(nomes_particoes);

    return comparacoes;
}


/*
 * Função principal (orquestradora) que o usuário chamará pelo menu.
 */
void ordenar_usuarios_selecao_natural() {
    FILE* arq = fopen("usuarios.dat", "rb");
    if (!arq) {
        printf("Base de dados de usuarios vazia ou nao existe.\n");
        return;
    }
    
    char log_msg[200];
    clock_t inicio, fim;
    double tempo_decorrido;

    printf("\nIniciando ordenacao externa para Usuarios...\n");
    printf("ETAPA 1: Gerando particoes com Selecao Natural...\n");

    // --- Executa a Seleção Natural ---
    inicio = clock();
    int num_particoes = selecao_natural_usuarios(arq);
    fim = clock();
    fclose(arq);

    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    if (num_particoes == -1) return;

    // --- Imprime e registra o log da ETAPA 1 ---
    printf("\n*****************************************\n");
    printf("Selecao Natural - TUsuario:\n");
    printf("Numero de particoes criadas: %d\n", num_particoes);
    printf("Tempo de execucao: %.2f segundos\n", tempo_decorrido);
    printf("*****************************************\n");
    
    // Prepara a mensagem de log
    sprintf(log_msg, "Selecao Natural - TUsuario: Particoes criadas: %d. Tempo: %.4f s.", num_particoes, tempo_decorrido);
    
    // ===== INÍCIO: Bloco de código para registrar em log_classificacao.log =====
    { // Usamos chaves para limitar o escopo das variáveis de log
        FILE* log_file = fopen("log_selecao.log", "a");
        if (log_file != NULL) {
            time_t agora = time(NULL);
            char* data_formatada = ctime(&agora);
            data_formatada[strcspn(data_formatada, "\n")] = 0; // Remove a quebra de linha
            fprintf(log_file, "[%s] %s\n", data_formatada, log_msg);
            fclose(log_file);
        }
    }
    // ===== FIM: Bloco de código do log =====


    if (num_particoes <= 1) {
        printf("\nArquivo de usuarios ja ordenado ou vazio.\n");
        return;
    }

    printf("\nETAPA 2: Intercalando particoes...\n");

    // --- Executa a Intercalação Ótima ---
    inicio = clock();
    int comparacoes = intercalacao_otima_usuarios(num_particoes);
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // --- Imprime e registra o log da ETAPA 2 ---
    printf("\n*****************************************\n");
    printf("Intercalacao Otima - TUsuario:\n");
    printf("Numero de comparacoes: %d\n", comparacoes);
    printf("Tempo de execucao: %.2f segundos\n", tempo_decorrido);
    printf("*****************************************\n");

    // Prepara a mensagem de log
    sprintf(log_msg, "Intercalacao Otima - TUsuario: Comparacoes: %d. Tempo: %.4f s.", comparacoes, tempo_decorrido);
    
    // ===== INÍCIO: Bloco de código para registrar em log_intercalacao.log =====
    {
        FILE* log_file = fopen("log_intercalacao.log", "a");
        if (log_file != NULL) {
            time_t agora = time(NULL);
            char* data_formatada = ctime(&agora);
            data_formatada[strcspn(data_formatada, "\n")] = 0;
            fprintf(log_file, "[%s] %s\n", data_formatada, log_msg);
            fclose(log_file);
        }
    }
    // ===== FIM: Bloco de código do log =====

    printf("\nArquivo de usuarios ordenado com sucesso!\n");
}