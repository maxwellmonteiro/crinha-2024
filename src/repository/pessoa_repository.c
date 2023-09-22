#define _GNU_SOURCE

#include "pessoa_repository.h"
#include "../util/db_connection.h"
#include "../util/string_util.h"
#include "../util/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAX_RECORDS 50

Pessoa *pessoa_repo_find_one(char *id) {
    Pessoa *pessoa = NULL;
    char *query;

    int ret = asprintf(&query, "select id, txt_apelido, txt_nome, dat_nascimento, txt_stack from pessoa where id = '%s'", id);
    if (ret > 121 || ret < 0) {
        log_error("out of bound (%d)", ret);
        exit(EXIT_FAILURE);
    }
    DBResultSet *result_set = db_query(db_get_connection(), query);
    free(query);

    if (result_set != NULL && result_set->rows_count > 0) {
        pessoa = malloc(sizeof(Pessoa));
        strzcpy(pessoa->id, result_set->records[0].fields_value[0], PESSOA_ID_LEN);
        strzcpy(pessoa->apelido, result_set->records[0].fields_value[1], UTF8_PESSOA_APELIDO_LEN);
        strzcpy(pessoa->nome, result_set->records[0].fields_value[2], UTF8_PESSOA_NOME_LEN);
        strzcpy(pessoa->nascimento, result_set->records[0].fields_value[3], PESSOA_NASCIMENTO_LEN);
        strzcpy(pessoa->stack, result_set->records[0].fields_value[4], PESSOA_STACKS_LEN);
    }
    db_clear_result_set(result_set);
    
    return pessoa;
}

PessoaList pessoa_repo_find_by_termo(char *termo) {
    PessoaList pessoas;
    char *query;

    string_util_tolower(termo);
    int ret = asprintf(&query, "select id, txt_apelido, txt_nome, dat_nascimento, txt_stack from pessoa " 
        "where busca_trgm like '%%%s%%' limit %d", termo, MAX_RECORDS);
    DBResultSet *result_set = db_query(db_get_connection(), query);
    free(query);

    if (result_set != NULL && result_set->rows_count > 0) {
        pessoas.values = malloc(sizeof(Pessoa) * result_set->rows_count);
        pessoas.size = result_set->rows_count;
        for (int i = 0; i < result_set->rows_count; i++) {
            strzcpy(pessoas.values[i].id, result_set->records[i].fields_value[0], PESSOA_ID_LEN);
            strzcpy(pessoas.values[i].apelido, result_set->records[i].fields_value[1], UTF8_PESSOA_APELIDO_LEN);
            strzcpy(pessoas.values[i].nome, result_set->records[i].fields_value[2], UTF8_PESSOA_NOME_LEN);
            strzcpy(pessoas.values[i].nascimento, result_set->records[i].fields_value[3], PESSOA_NASCIMENTO_LEN);
            strzcpy(pessoas.values[i].stack, result_set->records[i].fields_value[4], PESSOA_STACKS_LEN);
        }
    } else {
        pessoas.size = 0;
        pessoas.values = NULL;
    }
    db_clear_result_set(result_set);
    
    return pessoas;
}

void pessoa_repo_free_list(PessoaList pessoas) {
    if (pessoas.values != NULL) {
        free(pessoas.values);
    }
}

u_int64_t pessoa_repo_count() {
    u_int64_t count = 0;
    DBResultSet *result_set = db_query(db_get_connection(), "select count(id) from pessoa");

    if (result_set != NULL && result_set->rows_count > 0) {
        count = (u_int64_t) strtol(result_set->records[0].fields_value[0], NULL, 10);
    }
    db_clear_result_set(result_set);
    
    return count;
}

bool pessoa_repo_insert(Pessoa *pessoa) {
    char *query;

    int ret = asprintf(&query, 
        "insert into pessoa(id, txt_apelido, txt_nome, dat_nascimento, txt_stack) values('%s', '%s', '%s', '%s', '%s')",
        pessoa->id, pessoa->apelido, pessoa->nome, pessoa->nascimento, pessoa->stack);

    bool status = db_execute(db_get_connection(), query);
    free(query);

    if (!status) {
       strcpy(pessoa->id, "");
       return false;
    }
    return true;
}