#include "../entity/transacao.h"
#include <stdbool.h>
#include <inttypes.h>

extern TransacaoList transacao_repo_find_last_10(uint32_t cliente_id);
extern bool transacao_repo_insert(Transacao *transacao);
extern void transacao_repo_free_list(TransacaoList transacoes);