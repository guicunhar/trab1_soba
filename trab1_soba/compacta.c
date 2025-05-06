#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


// Calcula o tamanho de uma struct com base no descritor
int TamanhoStruct(char* descritor) {
    int tamanho = 0;
    int d = 0;

    while (descritor[d] != '\0') {
        char tipo = descritor[d];

        if (tipo == 'i') {
            tamanho += sizeof(int);
            d += 1;
        }
        else if (tipo == 'u') {
            tamanho += sizeof(unsigned int);
            d += 1;
        }
        else if (tipo == 's') {
            char tam_str[3] = { descritor[d + 1], descritor[d + 2], '\0' };
            int tam = atoi(tam_str);
            tamanho += tam;
            d += 3;
        }
        else {
            fprintf(stderr, "Tipo inválido no descritor: %c\n", tipo);
            return -1;
        }
    }

    return tamanho;
}

// Grava um inteiro no arquivo (com ou sem sinal)
void grava_inteiro(FILE* arquivo, int valor, int com_sinal) {
    if (com_sinal) {
        fwrite(&valor, sizeof(int), 1, arquivo);
    } else {
        unsigned int uvalor = (unsigned int)valor;
        fwrite(&uvalor, sizeof(unsigned int), 1, arquivo);
    }
}

// Grava uma string de tamanho fixo no arquivo
void grava_string(FILE* arquivo, char* valor, int tamanho) {
    fwrite(valor, sizeof(char), tamanho, arquivo);
}

// Função principal de gravação compacta
int gravacomp(int nstructs, void* valores, char* descritor, FILE* arquivo) {
    fputc(nstructs, arquivo); // grava a quantidade como 1 byte

    unsigned char* base = (unsigned char*)valores;
    int tamanhoStruct = TamanhoStruct(descritor);

    if (tamanhoStruct == -1) {
        return 0;
    }

    for (int i = 0; i < nstructs; i++) {
        int offset = 0;
        int d = 0;

        while (descritor[d] != '\0') {
            char tipo = descritor[d];

            if (tipo == 'i') {
                int* valor = (int*)(base + i * tamanhoStruct + offset);
                grava_inteiro(arquivo, *valor, 1);
                offset += sizeof(int);
                d += 1;
            }
            else if (tipo == 'u') {
                unsigned int* valor = (unsigned int*)(base + i * tamanhoStruct + offset);
                grava_inteiro(arquivo, *valor, 0);
                offset += sizeof(unsigned int);
                d += 1;
            }
            else if (tipo == 's') {
                char tam_str[3] = { descritor[d + 1], descritor[d + 2], '\0' };
                int tam = atoi(tam_str);
                char* valor = (char*)(base + i * tamanhoStruct + offset);
                grava_string(arquivo, valor, tam);
                offset += tam;
                d += 3;
            }
            else {
                fprintf(stderr, "Erro: tipo inválido no descritor: %c\n", tipo);
                return 0;
            }
        }
    }

    return 1;
}


// Lê um inteiro (assinado ou não) do arquivo, dado o tamanho em bytes, e retorna num int64_t
static int64_t le_valor(FILE* f, int tamanho, int com_sinal) {
    uint8_t buf[4] = { 0,0,0,0 };

    // Para big endian, preenche buf[4-tamanho .. 3]
    for (int i = 0; i < tamanho; i++) {
        buf[4 - tamanho + i] = fgetc(f);
    }

    if (com_sinal) {
        // Sign-extend manualmente
        int32_t v = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
        return v;
    } else {
        uint32_t v = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
        return v;
    }
}

static void mostracomp(FILE* arquivo) {
    int n = fgetc(arquivo);
    printf("Estruturas: %d\n\n", n);

    for (int i = 0; i < n; i++) {
        int valor1;
        unsigned int valor2;
        int valor3;
        char texto[11]; // +1 para o caractere nulo

        fread(&valor1, sizeof(int), 1, arquivo);
        fread(&valor2, sizeof(unsigned int), 1, arquivo);
        fread(&valor3, sizeof(int), 1, arquivo);
        fread(texto, sizeof(char), 10, arquivo);
        texto[10] = '\0'; // Garante a terminação da string

        printf("Estrutura %d:\n", i + 1);
        printf("  valor1: %d\n", valor1);
        printf("  valor2: %u\n", valor2);
        printf("  valor3: %d\n", valor3);
        printf("  texto: %s\n\n", texto);
    }
}


// Struct genérica para o exemplo com o descritor "iuis10"
struct Registro {
    int valor1;
    unsigned int valor2;
    int valor3;
    char texto[10];  // exatamente 10 bytes
};

int main() {
    struct Registro dados[2] = {
        { 42, 1000, -7, "TesteUm" },
        { -123, 9999, 88, "OutroTxt" }
    };
    // preenche \0
    for (int i = 0; i < 2; i++) {
        int len = strlen(dados[i].texto);
        for (int j = len; j < 10; j++) dados[i].texto[j] = 0;
    }

    // grava
    FILE* f = fopen("saida.bin", "wb");
    if (!f) { perror("fopen"); return 1; }
    if (!gravacomp(2, dados, "iuis10", f)) { fprintf(stderr, "Erro!\n"); }
    fclose(f);

    // mostra
    f = fopen("saida.bin", "rb");
    if (!f) { perror("fopen"); return 1; }
    mostracomp(f);
    fclose(f);

    return 0;
}
