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

    int ret = asprintf(&query, "select id, id_cliente, valor, tipo, descricao, to_char(realizada_em, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') from transacao "
        "where id_cliente = %d order by realizada_em desc limit 10", id_cliente);

    if (ret < 0) {
        log_error("Falha ao montar comando SQL");
        return transacoes;
    }

    DBResultSet *result_set = db_query(db_get_connection(), query);
    free(query);

    if (result_set != NULL && result_set->rows_count > 0) {
        transacoes.values = malloc(sizeof(Transacao) * result_set->rows_count);
        transacoes.size = result_set->rows_count;
        for (int i = 0; i < result_set->rows_count; i++) {
            transacoes.values[i].id = atoi(result_set->records[i].fields_value[0]);
            transacoes.values[i].id_cliente = atoi(result_set->records[i].fields_value[1]);
            transacoes.values[i].valor = atol(result_set->records[i].fields_value[2]);
            strzcpy(transacoes.values[i].tipo, result_set->records[i].fields_value[3], TRANSACAO_TIPO_LEN);
            strzcpy(transacoes.values[i].descricao, result_set->records[i].fields_value[4], UTF8_TRANSACAO_DESCRICAO_LEN);
            strzcpy(transacoes.values[i].realizada_em, result_set->records[i].fields_value[5], TRANSACAO_REALIZADA_EM_LEN);
        }
    } else {
        transacoes.size = 0;
        transacoes.values = NULL;
    }
    if (result_set != NULL) {
        db_clear_result_set(result_set);
    }

    return transacoes;
}

Cliente *transacao_repo_insert(Transacao *transacao) {
    char *query;

    char operacao = transacao->tipo[0] == 'c' ? '+' : '-';

    int ret = asprintf(&query,
        "update cliente set saldo = saldo %c %ld where id = %d;"
        "insert into transacao(id_cliente, valor, tipo, descricao) values(%d, %ld, '%s', '%s');"
        "select * from cliente where id = %d;",
        operacao, transacao->valor, transacao->id_cliente,
        transacao->id_cliente, transacao->valor, transacao->tipo, transacao->descricao,        
        transacao->id_cliente);

    if (ret < 0) {
        log_error("Falha ao montar comando SQL");
        return false;
    }

    DBResultSet *result_set = db_query(db_get_connection(), query);
    free(query);

    Cliente *cliente = NULL;
    if (result_set != NULL && result_set->rows_count > 0) {
        cliente = malloc(sizeof(Cliente));
        if (result_set->rows_count > 0) {
            cliente->id = atoi(result_set->records[0].fields_value[0]);
            cliente->limite = atol(result_set->records[0].fields_value[1]);
            cliente->saldo = atol(result_set->records[0].fields_value[2]);
        }
    }
    if (result_set != NULL) {
        db_clear_result_set(result_set);
    }

    return cliente;
}

bool transacao_repo_insert_async(Transacao *transacao) {
    char *query;

    char operacao = transacao->tipo[0] == 'c' ? '+' : '-';

    int ret = asprintf(&query,
        "update cliente set saldo = saldo %c %ld where id = %d;"
        "insert into transacao(id_cliente, valor, tipo, descricao) values(%d, %ld, '%s', '%s');",
        operacao, transacao->valor, transacao->id_cliente,
        transacao->id_cliente, transacao->valor, transacao->tipo, transacao->descricao     
        );

    if (ret < 0) {
        log_error("Falha ao montar comando SQL");
        return false;
    }

    bool res = db_execute_async(db_get_connection(), query);
    free(query);

    return res;
}

void transacao_repo_free_list(TransacaoList transacoes) {
    if (transacoes.values != NULL) {
        free(transacoes.values);
    }
}