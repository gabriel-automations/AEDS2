#include "filme.h"
#include "emprestimo.h"
#include "usuario.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> // Para a função trunc
#include <time.h>

// Protótipo da função que pega o próximo código, declarada aqui pra organizar.
int get_next_codigo(const char *nome_arquivo, int tamanho_registro);


// Esta função calcula o tamanho exato de cada registro de filme.
// Decidimos que precisava ser um valor fixo para podermos usar o fseek e pular de um registro para o outro.
int tamanho_registro_filme() {
    return sizeof(int)      // codigo
       + sizeof(char) * 100 // titulo
       + sizeof(int);     // ano
}

// Uma função de ajuda para criar um filme novo, pra gente não ter que repetir esse código.
// Ela aloca a memória e já preenche com os dados que passamos.
TFilme* filme(int cod, char* titulo, int ano) {
    TFilme* filme = (TFilme*) malloc(sizeof(TFilme));
    // O memset limpa qualquer "lixo" de memória. Achamos uma boa prática.
    if (filme) memset(filme, 0, sizeof(TFilme));
    filme->codigo = cod;
    strcpy(filme->titulo, titulo);
    filme->ano = ano;

    return filme;
}

// Salva um filme no arquivo binário.
// O fwrite escreve os dados "crus", por isso o arquivo final não é um texto legível.
void salva_filme(TFilme* filme, FILE* out) {
    fwrite(&filme->codigo, sizeof(int), 1, out);
    // Tivemos que garantir que o título sempre ocupe 100 caracteres para manter o tamanho fixo do registro.
    fwrite(filme->titulo, sizeof(char), sizeof(filme->titulo), out);
    fwrite(&filme->ano, sizeof(int), 1, out);

}

// Lê um filme do arquivo, fazendo o processo inverso da função de salvar.
TFilme* le_filme(FILE* in) {
    TFilme* filme = (TFilme*) malloc(sizeof(TFilme));
    // Precisamos checar o retorno do fread. Se for 0 ou menos, chegamos no fim do arquivo (EOF).
    if (fread(&filme->codigo, sizeof(int), 1, in) <= 0) {
        // Importante: se a leitura não deu certo, temos que liberar a memória que alocamos para não vazar.
        free(filme);
        return NULL;
    }
    // Se conseguiu ler o código, lê o resto dos campos.
    fread(filme->titulo, sizeof(char), sizeof(filme->titulo), in);
    fread(&filme->ano, sizeof(int), 1, in);

    return filme;
}

// Função simples para imprimir os dados de um filme de forma organizada na tela.
void imprime_filme(TFilme* filme) {
    // Usamos o operador ternário `? :` como um if/else mais compacto.
   printf("Cod: %d, Titulo: %s, Ano: %d\n",
       filme->codigo, filme->titulo, filme->ano);
}


// Descobre quantos filmes tem no arquivo.
// A lógica: vai até o fim do arquivo, pega o tamanho total em bytes e divide pelo tamanho de um filme.
int tamanho_arquivo_filmes(FILE* arq) {
    fseek(arq, 0, SEEK_END);
    int tam = trunc(ftell(arq) / tamanho_registro_filme());
    return tam;
}

// Busca um filme lendo o arquivo do início ao fim.
// É o método mais simples, mas pode ser lento em arquivos grandes.
TFilme* busca_sequencial_filme(FILE* arq, int codigo, long* pos, int* contador_comparacoes) {
    rewind(arq); 
    TFilme* filme;

    while ((filme = le_filme(arq)) != NULL) {
        (*contador_comparacoes)++; // Incrementa a cada filme lido e comparado
        if (filme->codigo == codigo) {

            *pos = ftell(arq) - tamanho_registro_filme();
            return filme;
        }

        free(filme);
    }
    *pos = -1; 
    return NULL;
}

/*
 * Busca binária.
 * ATENÇÃO: ESSA BUSCA SÓ FUNCIONA SE O ARQUIVO ESTIVER ORDENADO POR CÓDIGO!!!
 * É muito mais rápida porque vai "pulando" pelo arquivo.
 */
TFilme* busca_binaria_filme(FILE* arq, int codigo, long* pos, int* contador_comparacoes) {
    int num_registros = tamanho_arquivo_filmes(arq);
    int inicio = 0, fim = num_registros - 1;
    TFilme* filme = NULL;

    while (inicio <= fim) {
        (*contador_comparacoes)++; // Incrementa para cada iteração do loop
        int meio = inicio + (fim - inicio) / 2;
        *pos = meio * tamanho_registro_filme();
        fseek(arq, *pos, SEEK_SET);

        filme = le_filme(arq);
        if (!filme) break;

        (*contador_comparacoes)++; // Incrementa para a comparação de igualdade
        if (filme->codigo == codigo) {
            return filme;
        }
        (*contador_comparacoes)++; // Incrementa para a comparação de menor/maior
        if (filme->codigo < codigo) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
        free(filme);
    }
    *pos = -1;
    return NULL;
}

// Função que fizemos para ordenar o arquivo usando Selection Sort, direto no disco.
// É mais lenta que um sort em memória porque mexe muito com o arquivo (muitos acessos).
// Deixamos ela genérica, recebendo uma função `comparador` pra decidir como ordenar.
void selection_sort_disco_filme(FILE* arq, int (*comparador)(TFilme*, TFilme*), int* contador_comparacoes) {
    int num_registros = tamanho_arquivo_filmes(arq);
    rewind(arq);

    for (int i = 0; i < num_registros - 1; i++) {
        // Encontra a posição do menor elemento no resto do arquivo
        long pos_min = i * tamanho_registro_filme();
        fseek(arq, pos_min, SEEK_SET);
        TFilme* f_min = le_filme(arq);

        for (int j = i + 1; j < num_registros; j++) {
            long pos_atual = j * tamanho_registro_filme();
            fseek(arq, pos_atual, SEEK_SET);
            TFilme* f_atual = le_filme(arq);
            (*contador_comparacoes)++;

            // A função `comparador` que decide se o `f_atual` é menor que o `f_min`.
            if (comparador(f_atual, f_min) < 0) {
                free(f_min); // achamos um novo mínimo, o antigo não serve mais.
                f_min = f_atual;
                pos_min = pos_atual;
            } else {
                free(f_atual); // se não for o mínimo, podemos descartar.
            }
        }

        // A parte da troca no arquivo foi a que deu mais trabalho.
        // Primeiro, lemos o filme que está na posição 'i'.
        long pos_i = i * tamanho_registro_filme();
        fseek(arq, pos_i, SEEK_SET);
        TFilme* f_i = le_filme(arq);

        // Colocamos o menor filme (f_min) na posição 'i'.
        fseek(arq, pos_i, SEEK_SET);
        salva_filme(f_min, arq);

        // Colocamos o filme que estava em 'i' (o f_i) na posição antiga do menor.
        fseek(arq, pos_min, SEEK_SET);
        salva_filme(f_i, arq);

        // Limpamos a memória usada na troca.
        free(f_i);
        free(f_min);
    }
}

// Funções que o `selection_sort_disco_filme` usa para comparar.
int compara_por_ano(TFilme* a, TFilme* b) {
    return a->ano - b->ano;
}

int compara_por_codigo_f(TFilme* a, TFilme* b) {
    return a->codigo - b->codigo;
}


// Funções mais simples que só chamam o sort principal com o comparador certo.
void ordenar_por_ano() {
    FILE* arq = fopen("filmes.dat", "r+b"); 
    if (!arq) { 
        printf("Base de dados de filmes vazia.\n"); 
        return; 
    }

    printf("Ordenando filmes por ano. Isso pode levar um tempo...\n");

    // Variáveis para medição
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a medição de tempo
    inicio = clock();

    // Chama a função de ordenação, passando o comparador de ano
    selection_sort_disco_filme(arq, compara_por_ano, &comparacoes);

    // Termina a medição de tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    printf("Ordenacao por ano concluida.\n");

    // Prepara e registra a mensagem de log
    sprintf(log_mensagem, "Ordenacao por ANO (Selection Sort em Disco): Concluida. Tempo: %.6f segundos. Comparacoes: %d.",
            tempo_decorrido, comparacoes);
    registra_log(log_mensagem);
    
    // Lista os filmes na nova ordem, como a função original fazia
    listar_filmes(); 

    /*
     * Reordena por código para manter a busca binária funcionando,
     * exatamente como no seu código original.
     */
    printf("\nReordenando por codigo para manter a busca binaria funcionando...\n");
    // Não vamos medir esta segunda ordenação para não poluir o log,
    // mas poderíamos se quiséssemos.
    ordena_filmes_por_codigo(arq); 
    fclose(arq);
}

void ordena_filmes_por_codigo(FILE* arq) {
    printf("Ordenando filmes por codigo. Isso pode levar um tempo...\n");

    // Variáveis para medição
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a medição de tempo
    inicio = clock();

    // Chama a nova versão da função de ordenação
    selection_sort_disco_filme(arq, compara_por_codigo_f, &comparacoes);

    // Termina a medição de tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    printf("Ordenacao concluida!\n");

    // Prepara e registra a mensagem de log
    sprintf(log_mensagem, "Ordenacao por CODIGO (Selection Sort em Disco): Concluida. Tempo: %.6f segundos. Comparacoes: %d.",
            tempo_decorrido, comparacoes);
    registra_log(log_mensagem);
}

//----------------------------------------------------------

// Cria uma base de dados para testes.
void criarBaseFilme(FILE *out, int tam) {
    int vet[tam];
    TFilme *fm;

    for (int i = 0; i < tam; i++) {
        vet[i] = i + 1;
    }

    // Embaralhamos os códigos para a base não vir ordenada.
    embaralhar_vetor_filme(vet, tam);

    printf("\nGerando a base de dados...\n");

    for (int i = 0; i < tam; i++) {
        fm = filme(vet[i], "Velozes e Furiosos", 2005);
        salva_filme(fm, out);
        free(fm); // Liberamos a memória do filme logo após salvar no arquivo.
    }
}

// Pega um vetor e bagunça a ordem dos números.
void embaralhar_vetor_filme(int* vet, int tam) {
    // srand(time(NULL)) garante que o embaralhamento seja diferente a cada execução.
    srand(time(NULL)); 

    int trocas = (tam * 60) / 100;

    for (int t = 0; t < trocas; t++) {
        // Sorteia duas posições e troca os valores entre elas.
        int i = rand() % tam;
        int j = rand() % tam;
        
        int tmp = vet[i];
        vet[i] = vet[j];
        vet[j]=tmp;
    }
}


// Funções que o usuário chama pelo menu.

void cadastrar_filme() {
    char titulo[100];
    int ano;
    printf("Digite o titulo do filme: ");
    scanf(" %[^\n]", titulo); // O espaço aqui é um truque pra limpar o buffer do teclado.
    printf("Digite o ano de publicacao: ");
    scanf("%d", &ano);

    int cod = get_next_codigo("filmes.dat", tamanho_registro_filme());
    TFilme* f = filme(cod, titulo, ano);

    FILE* out = fopen("filmes.dat", "ab"); // "ab" = append binary, para adicionar no fim.
    salva_filme(f, out);
    fclose(out);

    printf("Filme '%s' (cod: %d) cadastrado.\n", f->titulo, f->codigo);
    free(f);

}

// Função auxiliar para verificar e imprimir o status de empréstimo de um filme
void imprime_status_emprestimo(int cod_filme) {
    // Abre o arquivo de empréstimos para leitura binária
    FILE* arq_emp = fopen("emprestimos.dat", "rb");

    // Se o arquivo de empréstimos não existe, significa que nenhum filme está emprestado.
    if (!arq_emp) {
        printf("Status: Disponivel\n");
        return;
    }

    TEmprestimo* emp;
    int emprestado = 0; // "Bandeira" para marcar se encontramos o filme

    // Loop para ler cada registro do arquivo de empréstimos
    while ((emp = le_emprestimo(arq_emp)) != NULL) {
        // Se o código do filme no registro de empréstimo for o que procuramos...
        if (emp->codigo_filme == cod_filme) {
            printf("Status: Emprestado para o usuario de codigo %d.\n", emp->codigo_usuario);
            emprestado = 1; // Marca que o filme foi encontrado
            free(emp);      // Libera a memória
            break;          // Encontrou, pode parar de procurar
        }
        free(emp); // Libera a memória se não for o que procuramos
    }

    // Se o loop terminou e a "bandeira" não foi acionada, o filme está disponível.
    if (!emprestado) {
        printf("Status: Disponivel.\n");
    }

    // Fecha o arquivo
    fclose(arq_emp);
}

void buscar_filme_sequencial() {
    int codigo;
    printf("Digite o codigo do filme para busca sequencial: ");
    scanf("%d", &codigo);

    FILE* arq = fopen("filmes.dat", "rb");
    if (!arq) { printf("Base de dados de filmes vazia.\n"); return; }

    // Variáveis para medição de desempenho
    long pos;
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a contagem do tempo
    inicio = clock();

    // Chama a nova versão da função, passando o endereço do contador
    TFilme* filme = busca_sequencial_filme(arq, codigo, &pos, &comparacoes);

    // Termina a contagem do tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Prepara a mensagem de log
    if (filme) {
        printf("Filme encontrado:\n");
        imprime_filme(filme);
        imprime_status_emprestimo(filme->codigo);
        sprintf(log_mensagem, "Busca sequencial pelo filme de codigo %d: SUCESSO. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
        free(filme);
    } else {
        printf("Filme com codigo %d nao encontrado.\n", codigo);
        sprintf(log_mensagem, "Busca sequencial pelo filme de codigo %d: FALHA. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
    }

    // Registra o log e fecha o arquivo
    registra_log(log_mensagem);
    fclose(arq);

}

// Em filme.c

void buscar_filme_binaria() {
    int codigo;
    printf("Digite o codigo do filme para busca binaria: ");
    scanf("%d", &codigo);

    FILE* arq = fopen("filmes.dat", "rb");
    if (!arq) { 
        printf("Base de dados de filmes vazia.\n"); 
        // Aqui, seria bom registrar um log de aviso também, se quiser
        // registra_log("Tentativa de busca binaria com base de dados vazia.");
        return; 
    }

    // Variáveis para medição de desempenho
    long pos;
    int comparacoes = 0;
    clock_t inicio, fim;
    double tempo_decorrido;
    char log_mensagem[200];

    // Inicia a contagem do tempo
    inicio = clock();

    // Chama a nova versão da função, passando o endereço do contador
    TFilme* filme = busca_binaria_filme(arq, codigo, &pos, &comparacoes);

    // Termina a contagem do tempo
    fim = clock();
    tempo_decorrido = ((double)(fim - inicio)) / CLOCKS_PER_SEC;

    // Prepara a mensagem de log
    if (filme) {
        printf("Filme encontrado:\n");
        imprime_filme(filme);
        imprime_status_emprestimo(filme->codigo);
        sprintf(log_mensagem, "Busca binaria pelo filme de codigo %d: SUCESSO. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
        free(filme);
    } else {
        printf("Filme com codigo %d nao encontrado.\n", codigo);
        sprintf(log_mensagem, "Busca binaria pelo filme de codigo %d: FALHA. Tempo: %.6f segundos. Comparacoes: %d.",
                codigo, tempo_decorrido, comparacoes);
    }

    // Registra o log e fecha o arquivo
    // Para que esta linha funcione, a função registra_log precisa ser "conhecida" aqui.
    // A melhor forma é declarar o protótipo dela em um dos seus arquivos .h, como "main.h" se existisse,
    // ou "utils.h", e incluir este .h em filme.c
    // Por enquanto, como você inclui o filme.c no main.c, isso pode funcionar diretamente.
    registra_log(log_mensagem);
    fclose(arq);
    
}

int filme_ja_emprestado(int cod_filme) {
    FILE* arq_emp = fopen("emprestimos.dat", "rb");
    if (!arq_emp) {
        return 0; // Se o arquivo não existe, nenhum filme foi emprestado.
    }

    TEmprestimo* emp;
    while ((emp = le_emprestimo(arq_emp)) != NULL) {
        if (emp->codigo_filme == cod_filme) {
            fclose(arq_emp);
            free(emp);
            return 1; // Encontrou o filme no arquivo de empréstimos.
        }
        free(emp);
    }

    fclose(arq_emp);
    return 0; // Não encontrou o filme.
}

void emprestar_filme() {
    int cod_filme, cod_usuario;
    int dummy_comparacoes = 0;
    printf("Digite o codigo do filme a ser emprestado: ");
    scanf("%d", &cod_filme);
    printf("Digite o codigo do usuario: ");
    scanf("%d", &cod_usuario);

    // 1. Verificar se o filme existe
    FILE* arq_filmes = fopen("filmes.dat", "rb");
    if (!arq_filmes) { printf("ERRO: Base de dados de filmes vazia.\n"); return; }
    long pos_filme;
    TFilme* filme = busca_sequencial_filme(arq_filmes, cod_filme, &pos_filme, &dummy_comparacoes);
    fclose(arq_filmes); 
    if (!filme) {
        printf("ERRO: Filme com codigo %d nao encontrado.\n", cod_filme);
        return;
    }
    free(filme); 

    // 2. Verificar se o usuário existe
    FILE* arq_usuarios = fopen("usuarios.dat", "rb");
    dummy_comparacoes = 0;
    if (!arq_usuarios) { printf("ERRO: Base de dados de usuarios vazia.\n"); return; }
    long pos_usuario;
    TUser* usuario = busca_sequencial_usuario(arq_usuarios, cod_usuario, &pos_usuario, &dummy_comparacoes);
    fclose(arq_usuarios);
    if (!usuario) {
        printf("ERRO: Usuario com codigo %d nao encontrado.\n", cod_usuario);
        return;
    }
    free(usuario);

    // 3. Verificar se o filme já não está emprestado
    if (filme_ja_emprestado(cod_filme)) {
        printf("ERRO: O filme de codigo %d ja esta emprestado.\n", cod_filme);
        return;
    }

    // 4. Se todas as verificações passaram, criar o empréstimo
    FILE* arq_emprestimos = fopen("emprestimos.dat", "ab");
    if (!arq_emprestimos) {
        printf("ERRO: Nao foi possivel abrir o arquivo de emprestimos.\n");
        return;
    }

    time_t data_atual = time(NULL); 
    TEmprestimo* novo_emprestimo = emprestimo(cod_filme, cod_usuario, data_atual);
    salva_emprestimo(novo_emprestimo, arq_emprestimos);
    
    printf("Emprestimo do filme %d para o usuario %d realizado com sucesso!\n", cod_filme, cod_usuario);
    
    free(novo_emprestimo);
    fclose(arq_emprestimos);
}

void devolver_filme() {
    int cod_filme, cod_usuario;
    printf("Digite o codigo do filme a ser devolvido: ");
    scanf("%d", &cod_filme);
    printf("Digite o codigo do usuario que esta devolvendo: ");
    scanf("%d", &cod_usuario);

    // Variável para usar nas funções de busca sem se preocupar com o número de comparações
    int dummy_comparacoes = 0;

    // Valida a existência do filme
    FILE* arq_filmes = fopen("filmes.dat", "rb");
    if (!arq_filmes) {
        printf("ERRO: Base de dados de filmes vazia.\n");
        return;
    }
    long pos_filme;
    // Usa a função de busca que já existe em filme.c
    TFilme* filme = busca_sequencial_filme(arq_filmes, cod_filme, &pos_filme, &dummy_comparacoes);
    fclose(arq_filmes);
    if (!filme) {
        printf("ERRO: Filme com codigo %d nao encontrado.\n", cod_filme);
        return;
    }
    free(filme); // Libera a memória, pois só precisávamos confirmar a existência

    // Valida a existência do usuário
    FILE* arq_usuarios = fopen("usuarios.dat", "rb");
    if (!arq_usuarios) {
        printf("ERRO: Base de dados de usuarios vazia.\n");
        return;
    }
    long pos_usuario;
    // Usa a função de busca que já existe em usuario.c
    TUser* usuario = busca_sequencial_usuario(arq_usuarios, cod_usuario, &pos_usuario, &dummy_comparacoes);
    fclose(arq_usuarios);
    if (!usuario) {
        printf("ERRO: Usuario com codigo %d nao encontrado.\n", cod_usuario);
        return;
    }
    free(usuario); // Libera a memória

    // --- Lógica para remover o registro de empréstimo ---

    // Abre o arquivo de empréstimos original para leitura
    FILE* arq_emp = fopen("emprestimos.dat", "rb");
    if (!arq_emp) {
        printf("INFO: Nenhum emprestimo registrado. Nada a fazer.\n");
        return;
    }

    // Cria um arquivo temporário para escrita
    FILE* arq_temp = fopen("emprestimos.tmp", "wb");
    if (!arq_temp) {
        printf("ERRO: Nao foi possivel criar o arquivo temporario.\n");
        fclose(arq_emp);
        return;
    }

    TEmprestimo* emp;
    int encontrado = 0; // "Bandeira" para sabermos se encontramos o empréstimo

    // Lê cada empréstimo do arquivo original
    while ((emp = le_emprestimo(arq_emp)) != NULL) { // le_emprestimo de emprestimo.c
        // Se este for o empréstimo que queremos remover...
        if (emp->codigo_filme == cod_filme && emp->codigo_usuario == cod_usuario) {
            encontrado = 1; // ...marca como encontrado e NÃO o copia para o arquivo temporário.
        } else {
            // Se não for, copia o registro para o arquivo temporário
            salva_emprestimo(emp, arq_temp); // salva_emprestimo de emprestimo.c
        }
        free(emp); // Libera a memória do registro que acabamos de ler
    }

    // Fecha os dois arquivos
    fclose(arq_emp);
    fclose(arq_temp);

    // Verifica se o empréstimo foi encontrado para decidir o que fazer
    if (encontrado) {
        // Apaga o arquivo original antigo
        remove("emprestimos.dat");
        // Renomeia o arquivo temporário para ser o novo arquivo oficial
        rename("emprestimos.tmp", "emprestimos.dat");
        printf("Filme %d devolvido com sucesso!\n", cod_filme);
    } else {
        // Se o empréstimo não foi encontrado, não precisamos do arquivo temporário
        remove("emprestimos.tmp");
        printf("ERRO: Nao foi encontrado um emprestimo do filme %d para o usuario %d.\n", cod_filme, cod_usuario);
    }
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