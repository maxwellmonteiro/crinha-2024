#include <inttypes.h>

#ifndef PESSOA_H
#define PESSOA_H

#define PESSOA_ID_LEN 36
#define PESSOA_APELIDO_LEN 32
#define PESSOA_NOME_LEN 100
#define PESSOA_NASCIMENTO_LEN 10
#define PESSOA_STACKS_LEN 512
#define PESSOA_EACH_STACK_LEN 32
#define UTF8_PESSOA_APELIDO_LEN (PESSOA_APELIDO_LEN * 2)
#define UTF8_PESSOA_NOME_LEN (PESSOA_NOME_LEN * 2)


typedef struct Pessoa {
    char id[PESSOA_ID_LEN + 1];
    char apelido[UTF8_PESSOA_APELIDO_LEN + 1];
    char nome[UTF8_PESSOA_NOME_LEN + 1];
    char nascimento[PESSOA_NASCIMENTO_LEN + 1];
    char stack[PESSOA_STACKS_LEN + 1];
} Pessoa;

typedef struct PessoaList {
    uint32_t size;
    Pessoa *values;    
} PessoaList;

#endif