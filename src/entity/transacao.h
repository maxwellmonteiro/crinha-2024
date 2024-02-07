#include <inttypes.h>

#ifndef TRANSACAO_H
#define TRANSACAO_H

#define TRANSACAO_ID_LEN 36
#define TRANSACAO_DESCRICAO_LEN 10
#define TRANSACAO_REALIZADA_EM_LEN 40
#define UTF8_TRANSACAO_DESCRICAO_LEN (TRANSACAO_DESCRICAO_LEN * 2)

typedef struct Transacao {
    char id[TRANSACAO_ID_LEN + 1];
    uint32_t id_cliente;
    int64_t valor;
    char tipo;
    char descricao[UTF8_TRANSACAO_DESCRICAO_LEN + 1];
    char realizada_em[TRANSACAO_REALIZADA_EM_LEN + 1];
} Transacao;

typedef struct TransacaoList {
    uint32_t size;
    Transacao *values;    
} TransacaoList;

#endif