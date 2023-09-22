#include "config_parser.h"
#include "log.h"
#include <confuse.h>

bool config_parser_init(Config *config, char *config_file) {
	cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("database-connection", &config->conn_str),
        CFG_SIMPLE_INT("server-port", &config->port),
		CFG_END()
	};
    int status = CFG_FAIL;
	cfg_t *cfg;
    cfg = cfg_init(opts, 0);
    if (cfg != NULL) {
	    status = cfg_parse(cfg, config_file);
        cfg_free(cfg);
    }
    return status == CFG_SUCCESS ? true : false;
}
