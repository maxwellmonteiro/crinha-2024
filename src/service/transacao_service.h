#include "../entity/transacao.h"
#include <stdbool.h>
#include <inttypes.h>

extern TransacaoList transacao_service_find_last_10(uint32_t cliente_id);
extern bool transacao_service_save(Transacao *transacao);
extern void transacao_service_free_list(TransacaoList transacoes);