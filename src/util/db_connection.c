#include "db_connection.h"
#include "../util/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static PGconn *pool = NULL;

int db_get_lib_version() {
    return PQlibVersion();
}

int db_get_server_version(PGconn *conn) {   
    return PQserverVersion(conn);
}

void db_add_to_pool(PGconn *conn) {
    if (pool == NULL && conn != NULL) {
        pool = conn;
    }
}

void db_init_pool(char *connet_str) {
    PGconn *conn = db_connect(connet_str);
    db_add_to_pool(conn);
}

void db_close_pool() {
    if (pool != NULL) {
        db_disconnect(pool);
    }
}

PGconn *db_get_connection() {
    int status = PQstatus(pool);
    if (status != CONNECTION_OK) {
        log_warn("Redefinindo conexão...");
        PQreset(pool);
    }
    return pool;
}

PGconn *db_connect(char *connet_str) {
    PGconn *conn = PQconnectdb(connet_str);
    while (PQstatus(conn) == CONNECTION_BAD) {        
        log_warn("Falha ao conectar ao banco de dados (%s)", PQerrorMessage(conn));
        sleep(1);      
        log_warn("Tentando nova conexão ...");  
        conn = PQconnectdb(connet_str);
        // PQfinish(conn);
        // exit(EXIT_FAILURE);
    }
    log_info("Conectado ao banco de dados");
    return conn;
}

void db_disconnect(PGconn * conn) {
    PQfinish(conn);
}

DBResultSet *db_query(PGconn *conn, char *query) {
    DBResultSet *result_set = NULL;
    PGresult *res = PQexec(conn, query);

    int status = PQresultStatus(res);
    if (status == PGRES_TUPLES_OK) {
        int rows = PQntuples(res);
        int fields = PQnfields(res);
        result_set = db_create_result_set(res, rows, fields);
    } else {
        log_debug("Falha ao executar query (%s)", PQresultErrorMessage(res));
    }  
    PQclear(res);   
    return result_set; 
}

bool db_execute(PGconn *conn, char *query) {
    bool ret = true;
    DBResultSet *result_set = NULL;
    PGresult *res = PQexec(conn, query);
        
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        log_debug("Falha ao executar comando (%s)", PQresultErrorMessage(res));
        ret = false;
    }  
    PQclear(res);
    return ret; 
}

DBResultSet *db_create_result_set(PGresult *res, int rows, int fields) {
    DBResultSet *result_set = NULL;
    result_set = malloc(sizeof(DBResultSet));
    result_set->rows_count = rows;
    result_set->fields_count = fields;
    if (rows > 0) {
        result_set->records = malloc(sizeof(DBRecord) * rows);
    } else {
        result_set->records = NULL;
    }
    for (int i = 0; i < rows; i++) {
        result_set->records[i].fields_value = malloc(sizeof(char *) * fields);
        for (int j = 0; j < fields; j++) {
            db_copy_field(result_set, i, j, PQgetvalue(res, i, j));
        }
    }
    return result_set;
}

inline void db_copy_field(DBResultSet *result_set, int row_idx, int field_idx, char *value) {
    result_set->records[row_idx].fields_value[field_idx] = malloc(sizeof(char) * (strlen(value) + 1));
    strcpy(result_set->records[row_idx].fields_value[field_idx], value);
}

void db_clear_result_set(DBResultSet *result_set) {
    for (int i = 0; i < result_set->rows_count; i++) {
        for (int j = 0; j < result_set->fields_count; j++) {
            free(result_set->records[i].fields_value[j]);
            result_set->records[i].fields_value[j] = NULL;
        }
        free(result_set->records[i].fields_value);
        result_set->records[i].fields_value = NULL;
    }
    if (result_set->records != NULL) {
        free(result_set->records);
    }
    result_set->records = NULL;
    free(result_set);    
}