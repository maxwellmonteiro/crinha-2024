#include "env_util.h"
#include <stdlib.h>
#include "log.h"

char *env_util_get(const char* name) {
    char *ret = getenv(name);

    if (ret == NULL) {
        log_error("Variável de ambiente não encontrada %s", name);
    }
    return ret;
}