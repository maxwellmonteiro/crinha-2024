#include "util/db_connection.h"
#include "server/server.h"
#include "controller/pessoa_controller.h"
#include "util/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_DB_CONNECTION_STR "DB_CONNECTION_STR"
#define ENV_PORT "PORT"

int main(int argc, char **argv) {

    log_set_level(LOG_INFO);

    db_init_pool(getenv(ENV_DB_CONNECTION_STR));

    pessoa_controller_init();

    server_init(atoi(getenv(ENV_PORT)));
    
    db_close_pool();

    return 0;
}