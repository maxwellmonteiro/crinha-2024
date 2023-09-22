#include "../entity/pessoa.h"
#include <stdbool.h>
#include <inttypes.h>

extern Pessoa *pessoa_repo_find_one(char *id);
extern PessoaList pessoa_repo_find_by_termo(char *termo);
extern bool pessoa_repo_insert(Pessoa *pessoa);
extern uint64_t pessoa_repo_count();
extern void pessoa_repo_free_list(PessoaList pessoas);
