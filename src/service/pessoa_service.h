#include "../entity/pessoa.h"
#include <stdbool.h>
#include <inttypes.h>

extern Pessoa *pessoa_service_find_by_id(char *id);
extern PessoaList pessoa_service_find_by_termo(char *termo);
extern bool pessoa_service_save(Pessoa *entity);
extern uint64_t pessoa_service_count();
extern void pessoa_service_free_list(PessoaList pessoas);