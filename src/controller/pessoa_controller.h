
extern void pessoa_controller_init();
char *pessoa_controller_save(const char *url, const char *body);
char *pessoa_controller_find_by_id(const char *url);
char *pessoa_controller_find_by_termo(const char *url);
char *pessoa_controller_count(const char *url);