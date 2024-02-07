#define _GNU_SOURCE

#include "cliente_repository.h"
#include "../util/db_connection.h"
#include "../util/string_util.h"
#include "../util/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#define MAX_RECORDS 50

Cliente *cliente_repo_find_one(uint32_t id) {
    Cliente *cliente = NULL;
    char *query;

    int ret = asprintf(&query, "select id, limite, saldo from cliente where id = %d", id);
    if (ret < 0) {
        log_error("falha ao montar string insert sql");
        exit(EXIT_FAILURE);
    }
    DBResultSet *result_set = db_query(db_get_connection(), query);
    free(query);

    if (result_set != NULL && result_set->rows_count > 0) {
        cliente = malloc(sizeof(Cliente));
        cliente->id = atoi(result_set->records[0].fields_value[0]);
        cliente->limite = atol(result_set->records[0].fields_value[1]);
        cliente->saldo = atol(result_set->records[0].fields_value[2]);
    }
    db_clear_result_set(result_set);
    
    return cliente;
}
