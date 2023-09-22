#include <stdbool.h>

typedef struct Config {
  char *conn_str;
  long int port;
} Config;

extern bool config_parser_init(Config *config, char *config_file);