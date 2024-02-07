#define _GNU_SOURCE

#include "transacao_repository.h"
#include "../util/db_connection.h"
#include "../util/string_util.h"
#include "../util/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>


TransacaoList transacao_repo_find_last_10(uint32_t id_cliente) {
    TransacaoList transacoes;
    char *query;

    int ret = asprintf(&query, "select id, id_cliente, valor, tipo, descricao, realizada_em from transacao "
        "where id_cliente = %d order by realizada_em desc limit 10", id_cliente);
    DBResultSet *result_set = db_query(db_get_connection(), query);
    free(query);

    if (result_set != NULL && result_set->rows_count > 0) {
        transacoes.values = malloc(sizeof(Transacao) * result_set->rows_count);
        transacoes.size = result_set->rows_count;
        for (int i = 0; i < result_set->rows_count; i++) {
            strzcpy(transacoes.values[i].id, result_set->records[i].fields_value[0], TRANSACAO_ID_LEN);
            transacoes.values[i].id_cliente = atoi(result_set->records[i].fields_value[1]);
            transacoes.values[i].valor = atol(result_set->records[i].fields_value[2]);
            transacoes.values[i].tipo = result_set->records[i].fields_value[3][0];
            strzcpy(transacoes.values[i].descricao, result_set->records[i].fields_value[4], UTF8_TRANSACAO_DESCRICAO_LEN);
            strzcpy(transacoes.values[i].realizada_em, result_set->records[i].fields_value[5], TRANSACAO_REALIZADA_EM_LEN);
        }
    } else {
        transacoes.size = 0;
        transacoes.values = NULL;
    }
    db_clear_result_set(result_set);

    return transacoes;
}

bool transacao_repo_insert(Transacao *transacao) {
    char *query;

    char operacao = transacao->tipo == 'c' ? '+' : '-';

    asprintf(&query,
        "insert into transacao(id, id_cliente, valor, tipo, descricao) values('%s', %d, %ld, '%c', '%s');"
        "select * from cliente where id_cliente = %d for update;"
        "update cliente set saldo = saldo %c %ld where id_cliente = %d;",
        transacao->id, transacao->id_cliente, transacao->valor, transacao->tipo, transacao->descricao,
        transacao->id_cliente,
        operacao, transacao->valor, transacao->id_cliente);

    bool status = db_execute(db_get_connection(), query);
    free(query);

    if (!status) {
       strcpy(transacao->id, "");
       return false;
    }
    return true;
}

void transacao_repo_free_list(TransacaoList transacoes) {
    if (transacoes.values != NULL) {
        free(transacoes.values);
    }
}