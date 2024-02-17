#include <sys/time.h>

extern char *env_util_get(const char* name);
extern void timeval_print(struct timeval *tv);
extern void timeval_print_elapsed_if_greater(struct timeval *tv_begin, struct timeval *tv_end,  time_t seconds, suseconds_t micro_seconds, char *suffix);