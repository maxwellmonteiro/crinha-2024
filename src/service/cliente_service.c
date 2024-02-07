#include "cliente_service.h"
#include "../repository/cliente_repository.h"

Cliente *cliente_service_find_one(uint32_t id) {
    return cliente_repo_find_one(id);
}
