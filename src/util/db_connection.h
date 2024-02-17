#include <libpq-fe.h>
#include <stdbool.h>

typedef struct DBRecord {
    char **fields_value;
} DBRecord;

typedef struct DBResultSet {
    DBRecord *records;
    int rows_count;
    int fields_count;
} DBResultSet;

DBResultSet *db_create_result_set(PGresult *res, int rows, int fields);

extern void db_close_pool();

void db_copy_field(DBResultSet *result_set, int row_idx, int field_idx, char *value);

extern int db_get_lib_version();

extern int db_get_server_version(PGconn *conn);  

extern PGconn *db_connect();

extern void db_disconnect(PGconn *conn);

extern DBResultSet *db_query(PGconn *conn, char *query);

extern bool db_execute(PGconn *conn, char *query);

extern bool db_execute_async(PGconn *conn, char *query);

extern void db_clear_result_set(DBResultSet *result_set);

void db_init_pool(char *connet_str);

PGconn *db_get_connection();

