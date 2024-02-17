#include "../entity/transacao.h"
#include "../entity/cliente.h"
#include <stdbool.h>
#include <inttypes.h>

extern TransacaoList transacao_repo_find_last_10(uint32_t cliente_id);
extern Cliente *transacao_repo_insert(Transacao *transacao);
extern bool transacao_repo_insert_async(Transacao *transacao);
extern void transacao_repo_free_list(TransacaoList transacoes);