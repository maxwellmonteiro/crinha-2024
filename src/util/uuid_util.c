#include "uuid_util.h"
#include <uuid/uuid.h>
#include <stdlib.h>

void uuid_util_generate_random(char *uuid) {
    uuid_t binuuid;
    uuid_generate_time(binuuid);
    uuid_unparse_lower(binuuid, uuid);
}