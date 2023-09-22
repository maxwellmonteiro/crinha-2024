#include "util/db_connection.h"
#include "util/config_parser.h"
#include "server/server.h"
#include "controller/pessoa_controller.h"
#include "util/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


// void log_lock_fn(bool lock, void *udata) {
//     pthread_mutex_t *log_lock = (pthread_mutex_t *) udata;
//     if (lock) {
//         pthread_mutex_lock(log_lock);
//     } else {
//         pthread_mutex_unlock(log_lock);
//     }
// }

int main(int argc, char **argv) {

    // pthread_mutex_t log_lock;
    // pthread_mutex_init(&log_lock, NULL);

    log_set_level(LOG_INFO);
    // log_set_lock(log_lock_fn, &log_lock);

    Config config;
    if (!config_parser_init(&config, "./crinha.conf")) {
        log_fatal("Falha ao ler arquivo de configurações");
        exit(EXIT_FAILURE);
    }

    db_init_pool(config.conn_str);

    pessoa_controller_init();

    server_init(config.port);
    
    db_close_pool();

    // pthread_mutex_destroy(&log_lock);

    return 0;
}