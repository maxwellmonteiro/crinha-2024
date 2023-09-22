#include "pessoa_service.h"
#include "../repository/pessoa_repository.h"
#include "../util/uuid_util.h"
#include "../util/string_util.h"
#include <stdlib.h>
#include <string.h>

Pessoa *pessoa_service_find_by_id(char *id) {
    return pessoa_repo_find_one(id);
}

PessoaList pessoa_service_find_by_termo(char *termo) {
    return pessoa_repo_find_by_termo(termo);
}

bool pessoa_service_save(Pessoa *entity) {
    uuid_util_generate_random(entity->id);
    return pessoa_repo_insert(entity);
}

uint64_t pessoa_service_count() {
    return pessoa_repo_count();
}

void pessoa_service_free_list(PessoaList pessoas) {
    pessoa_repo_free_list(pessoas);
}