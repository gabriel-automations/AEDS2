#include "filme.h"
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
           + sizeof(int)      // ano
           + sizeof(int)      // disponivel
           + sizeof(int);     // codigo_usuario_emprestado
}

// Uma função de ajuda para criar um filme novo, pra gente não ter que repetir esse código.
// Ela aloca a memória e já preenche com os dados que passamos.
TFilme* filme(int cod, char* titulo, int ano, int disponivel, int cod_usuario) {
    TFilme* filme = (TFilme*) malloc(sizeof(TFilme));
    // O memset limpa qualquer "lixo" de memória. Achamos uma boa prática.
    if (filme) memset(filme, 0, sizeof(TFilme));
    filme->codigo = cod;
    strcpy(filme->titulo, titulo);
    filme->ano = ano;
    filme->disponivel = disponivel;
    filme->codigo_usuario_emprestado = cod_usuario;
    return filme;
}

// Salva um filme no arquivo binário.
// O fwrite escreve os dados "crus", por isso o arquivo final não é um texto legível.
void salva_filme(TFilme* filme, FILE* out) {
    fwrite(&filme->codigo, sizeof(int), 1, out);
    // Tivemos que garantir que o título sempre ocupe 100 caracteres para manter o tamanho fixo do registro.
    fwrite(filme->titulo, sizeof(char), sizeof(filme->titulo), out);
    fwrite(&filme->ano, sizeof(int), 1, out);
    fwrite(&filme->disponivel, sizeof(int), 1, out);
    fwrite(&filme->codigo_usuario_emprestado, sizeof(int), 1, out);
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
    fread(&filme->disponivel, sizeof(int), 1, in);
    fread(&filme->codigo_usuario_emprestado, sizeof(int), 1, in);
    return filme;
}

// Função simples para imprimir os dados de um filme de forma organizada na tela.
void imprime_filme(TFilme* filme) {
    // Usamos o operador ternário `? :` como um if/else mais compacto.
    printf("Cod: %d, Titulo: %s, Ano: %d, Disponivel: %s\n",
           filme->codigo, filme->titulo, filme->ano, filme->disponivel ? "Sim" : "Nao");
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
TFilme* busca_sequencial_filme(FILE* arq, int codigo, long* pos) {
    rewind(arq); // Garante que a busca comece do início.
    TFilme* filme;
    
    // Lê filme por filme até o arquivo acabar.
    while ((filme = le_filme(arq)) != NULL) {
        if (filme->codigo == codigo) {
            // Achamos! Um detalhe aqui: ftell() dá a posição DEPOIS da leitura,
            // então a gente volta o tamanho do registro para pegar o início certinho.
            *pos = ftell(arq) - tamanho_registro_filme();
            return filme;
        }
        // Lembrete para nós: se não é o filme que queremos, precisamos liberar a memória dele.
        free(filme);
    }
    *pos = -1; // Sinaliza que não encontrou.
    return NULL;
}

/*
 * Busca binária.
 * ATENÇÃO: ESSA BUSCA SÓ FUNCIONA SE O ARQUIVO ESTIVER ORDENADO POR CÓDIGO!!!
 * É muito mais rápida porque vai "pulando" pelo arquivo.
 */
TFilme* busca_binaria_filme(FILE* arq, int codigo, long* pos) {
    int num_registros = tamanho_arquivo_filmes(arq);
    int inicio = 0, fim = num_registros - 1;
    TFilme* filme = NULL;

    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;
        *pos = meio * tamanho_registro_filme();
        fseek(arq, *pos, SEEK_SET); // O "pulo" da busca binária acontece aqui.

        filme = le_filme(arq);
        if (!filme) break; // Segurança

        if (filme->codigo == codigo) {
            return filme; // Encontrado!
        }
        if (filme->codigo < codigo) {
            inicio = meio + 1; // Se o código do meio for menor, procuramos na metade da direita.
        } else {
            fim = meio - 1; // Se for maior, procuramos na metade da esquerda.
        }
        free(filme); // Liberamos o filme do "meio" se não for o que buscamos.
    }
    *pos = -1;
    return NULL;
}

// Função que fizemos para ordenar o arquivo usando Selection Sort, direto no disco.
// É mais lenta que um sort em memória porque mexe muito com o arquivo (muitos acessos).
// Deixamos ela genérica, recebendo uma função `comparador` pra decidir como ordenar.
void selection_sort_disco_filme(FILE* arq, int (*comparador)(TFilme*, TFilme*)) {
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
void ordena_filmes_por_ano(FILE* arq) {
    selection_sort_disco_filme(arq, compara_por_ano);
}

void ordena_filmes_por_codigo(FILE* arq) {
    printf("Ordenacao feita!!");
    selection_sort_disco_filme(arq, compara_por_codigo_f);
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
        fm = filme(vet[i], "Velozes e Furiosos", 2005, 1, 1);
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
    TFilme* f = filme(cod, titulo, ano, 1, -1);

    FILE* out = fopen("filmes.dat", "ab"); // "ab" = append binary, para adicionar no fim.
    salva_filme(f, out);
    fclose(out);

    printf("Filme '%s' (cod: %d) cadastrado.\n", f->titulo, f->codigo);
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
        printf("Filme encontrado:\n");
        imprime_filme(filme);
        free(filme);
    } else {
        printf("Filme com codigo %d nao encontrado.\n", codigo);
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
        printf("Filme encontrado:\n");
        imprime_filme(filme);
        free(filme);
    } else {
        printf("Filme com codigo %d nao encontrado.\n", codigo);
    }
    fclose(arq);

}

void ordenar_por_ano() {
    FILE* arq = fopen("filmes.dat", "r+b"); // "r+b" para ler e escrever no mesmo arquivo.
    if (!arq) { printf("Base de dados de filmes vazia.\n"); return; }

    printf("Ordenando filmes por ano...\n");
    ordena_filmes_por_ano(arq);
    printf("Ordenacao concluida.\n");
    
    listar_filmes(); 

    /*
     * Detalhe super importante que discutimos:
     * A busca binária que usamos no programa depende do arquivo estar ordenado por CÓDIGO.
     * Se a gente deixasse ordenado por ano, a busca ia parar de funcionar.
     * Por isso, depois de mostrar a lista por ano, nós ordenamos de volta por código
     * para não quebrar o resto do programa.
     */
    printf("\nReordenando por codigo para manter a busca binaria funcionando...\n");
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

    // Altera os dados do filme em memória
    filme->disponivel = 0;
    filme->codigo_usuario_emprestado = cod_usuario;

    // Aqui usamos a posição que a busca nos deu para voltar exatamente no lugar certo do arquivo...
    fseek(arq_filmes, pos, SEEK_SET);
    // ... e sobrescrever o registro com os dados novos.
    salva_filme(filme, arq_filmes);

    printf("Filme '%s' emprestado com sucesso.\n", filme->titulo);
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
        printf("ERRO: Filme nao encontrado.\n");
        fclose(arq_filmes);
        return;
    }
    if (filme->disponivel) {
        printf("ERRO: Filme '%s' ja estava disponivel.\n", filme->titulo);
        free(filme);
        fclose(arq_filmes);
        return;
    }
    
    filme->disponivel = 1;
    filme->codigo_usuario_emprestado = -1;

    // Mesmo esquema do 'emprestar': volta na posição certa e sobrescreve.
    fseek(arq_filmes, pos, SEEK_SET);
    salva_filme(filme, arq_filmes);

    printf("Filme '%s' devolvido com sucesso.\n", filme->titulo);
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