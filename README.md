# Locadora de Filmes - AEDS2
#### **Autores**: Gabriel Santos Caldeira e Rian Vaz Maurício

Projeto acadêmico da disciplina de Algoritmos e Estrutura de Dados II, consiste em um sistema de gerenciamento de locadora desenvolvido em C, com foco na manipulação e ordenação eficiente de grandes volumes de dados em arquivos externos.

---

## Os métodos comparados são:

* **Selection Sort**, adaptada para operar diretamente em disco.

* **Ordenação por Intercalação Ótima com Seleção Natural**, um método de Mergesort Externo de múltiplos passos.

## Registros de .log:

Os dados de performance foram extraídos dos arquivos de log gerados pela aplicação (`.log`).


### Selection Sort com 5000 usuarios
 ```
Ordenacao de USUARIOS por codigo (Selection Sort em Disco): Concluida. Tempo: 40.010000 segundos. Comparacoes: 12497500.

```

### Seleção Natural com Intercalação Ótima com 5000 usuarios
 ```
Selecao Natural - TUsuario: Particoes criadas: 384. Tempo: 0.4100 s.

Intercalacao Otima - TUsuario: Comparacoes: 1717947. Tempo: 2.5030 s.

```


## Análise de resuldados:

Fica evidente que, embora o **Selection Sort** seja mais simples de implementar, sua performance se degrada drasticamente com o aumento do volume de dados, tornando-o inviável para qualquer aplicação real que lide com arquivos. O seu padrão de acesso aleatório ao disco é o principal responsável pela lentidão.

Em contrapartida, a **Seleção Natural com Intercalação Ótima** demonstrou ser ordens de magnitude mais eficiente. Ao priorizar operações sequenciais de leitura e escrita e reduzir o número de "passadas" sobre os dados, o algoritmo consegue ordenar um grande volume de registros em uma fração do tempo. A complexidade maior de sua implementação é um preço justo a se pagar pela performance e escalabilidade obtidas.

Para os requisitos do projeto, que preveem uma base de dados que pode crescer, a **Seleção Natural com Intercalação Ótima** é, inquestionavelmente, o método de ordenação superior e recomendado.