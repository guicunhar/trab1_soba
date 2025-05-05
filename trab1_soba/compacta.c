#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

int TamanhoStruct(char* descritor) {
    int tamanho = 0;
    int d = 0;

    while (descritor[d] != '\0') {
        char tipo = descritor[d];

        if (tipo == 'i') {
            // Campo do tipo int (4 bytes, com sinal)
            tamanho += sizeof(int);
            d += 1;
        }
        else if (tipo == 'u') {
            // Campo do tipo unsigned int (4 bytes, sem sinal)
            tamanho += sizeof(unsigned int);
            d += 1;
        }
        else if (tipo == 's') {
            // Campo do tipo string, ex: s10
            char tam_str[3] = { descritor[d + 1], descritor[d + 2], '\0' };
            int tam = atoi(tam_str);
            tamanho += tam;
            d += 3;
        }
        else {
            // Tipo inválido no descritor
            fprintf(stderr, "T inválido no descritor: %c\n", tipo);
            return -1;
        }
    }

    return tamanho;
}

int gravacomp(int nstructs, void* valores, char* descritor, FILE* arquivo) {
    // Escreve o número de structs no arquivo (como 1 byte)
    fputc(nstructs, arquivo);

    // Converte ponteiro genérico para bytes
    unsigned char* base = (unsigned char*)valores;

    // Calcula o tamanho da struct a partir do descritor
    int tamanhoStruct = calcTamanhoStruct(descritor);
    if (tamanhoStruct == -1) {
        return 0; // Erro no cálculo do tamanho da struct
    }

    // Para cada struct
    for (int i = 0; i < nstructs; i++) {
        int offset = 0; // deslocamento dentro da struct
        int d = 0;      // índice no descritor

        // Para cada campo descrito
        while (descritor[d] != '\0') {
            char tipo = descritor[d];

            if (tipo == 'i') {
                // Campo do tipo int (4 bytes, com sinal)
                int* valor = (int*)(base + i * tamanhoStruct + offset);
                // chamada: grava_inteiro(arquivo, *valor, 1);
                offset += sizeof(int);
                d += 1;
            }
            else if (tipo == 'u') {
                // Campo do tipo unsigned int (4 bytes, sem sinal)
                unsigned int* valor = (unsigned int*)(base + i * tamanhoStruct + offset);
                // chamada: grava_inteiro(arquivo, *valor, 0);
                offset += sizeof(unsigned int);
                d += 1;
            }
            else if (tipo == 's') {
                // Campo do tipo string, ex: s10
                char tam_str[3] = { descritor[d + 1], descritor[d + 2], '\0' };
                int tam = atoi(tam_str);
                char* valor = (char*)(base + i * tamanhoStruct + offset);
                // chamada: grava_string(arquivo, valor, tam);
                offset += tam;
                d += 3;
            }
            else {
                // Tipo inválido no descritor
                fprintf(stderr, "Erro: tipo inválido no descritor: %c\n", tipo);
                return 0;
            }
        }
    }

    return 1; // sucesso
}