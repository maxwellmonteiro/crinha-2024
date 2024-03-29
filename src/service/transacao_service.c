
#include "transacao_service.h"
#include "../repository/transacao_repository.h"
#include "../util/uuid_util.h"
#include "../util/string_util.h"
#include <stdlib.h>
#include <string.h>

TransacaoList transacao_service_find_last_10(uint32_t cliente_id) {
    return transacao_repo_find_last_10(cliente_id);
}

Cliente *transacao_service_save(Transacao *transacao) {
    return transacao_repo_insert(transacao);
}

bool transacao_service_save_async(Transacao *transacao) {
    return transacao_repo_insert_async(transacao);
}

void transacao_service_free_list(TransacaoList transacoes) {
    transacao_repo_free_list(transacoes);
}