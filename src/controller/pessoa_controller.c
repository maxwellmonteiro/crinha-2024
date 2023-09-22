#include "pessoa_controller.h"
#include "../service/pessoa_service.h"
#include "../server/router.h"
#include "../server/router.h"
#include "../util/string_util.h"
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <inttypes.h>

#define __USE_XOPEN
#include <time.h>

#include <jansson.h>
#include "../util/log.h"

#define MAX_BUFFER_URL_PARAM 128
#define MAX_BUFFER_FORM_PARAM 256
#define MAX_BUFFER_CONTAGEM 128
#define MAX_BUFFER_FIND_BY_TERMO 20480
#define MAX_BUFFER_FIND_BY_ID 1024
#define MAX_BUFFER_SAVE 512

#define MATCH_EXACT(STR) ("^"STR"$")
#define MATCH_ONE_URL_PARAM(STR) ("^\\"STR"\\/([a-zA-Z0-9\\-]*|[^\\/])$")
#define MATCH_FORM_PARAM(STR) ("^\\"STR"\\?([^\\/]*)$")

static regex_t matcher_save;
static regex_t matcher_find_by_id;
static regex_t matcher_find_by_termo;
static regex_t matcher_count;

static Route route_save = { HTTP_POST, MATCH_EXACT("/pessoas"), NULL, pessoa_controller_save };
static Route route_find_by_id = { HTTP_GET, MATCH_ONE_URL_PARAM("/pessoas"), pessoa_controller_find_by_id, NULL };
static Route route_find_by_termo = { HTTP_GET, MATCH_FORM_PARAM("/pessoas"), pessoa_controller_find_by_termo, NULL };
static Route route_count = { HTTP_GET, MATCH_EXACT("/contagem-pessoas"), pessoa_controller_count, NULL };

static char buffer_url_param[MAX_BUFFER_URL_PARAM];
static char buffer_form_param[MAX_BUFFER_FORM_PARAM];
static char buffer_response_contagem[MAX_BUFFER_CONTAGEM];
static char buffer_response_find_by_termo[MAX_BUFFER_FIND_BY_TERMO];
static char buffer_response_find_by_id[MAX_BUFFER_FIND_BY_ID];
static char buffer_response_save[MAX_BUFFER_SAVE];

char *extract_param(char *buffer, const char *url, char delimiter, size_t size);
char *extract_url_param(const char *url);
char *extract_form_param(const char *url);
json_t *build_json_stacks(char *stacks);
json_t *build_json_pessoa(Pessoa *pessoa);
void build_string_stacks(char *stacks,  json_t *json_stacks, size_t size);
bool is_data_valida(const char *data);
bool is_stacks_validas(json_t *json_stacks, size_t size);

void pessoa_controller_init() {  
    // router_compile_matcher(&route_save);
    // router_compile_matcher(&route_find_by_id);
    // router_compile_matcher(&route_find_by_termo);
    // router_compile_matcher(&route_count);
    router_add_route(&route_save);
    router_add_route(&route_find_by_id);
    router_add_route(&route_find_by_termo);
    router_add_route(&route_count);
}

char response_save[] = "HTTP/1.1 201 Created""\r\n"
                            "Location: /pessoas/%s""\r\n"                            
                            "Content-Length: 0""\r\n"
                            "\r\n";
                            

char *pessoa_controller_save(const char *url, const char *body) {
    char *resp = HTTP_BAD_REQUEST;
    if (body != NULL) {
        json_t *json_pessoa = json_loads(body, 0, NULL);
        json_t *json_nascimento = json_object_get(json_pessoa, "nascimento");

        if (!is_data_valida(json_string_value(json_nascimento))) {
            json_decref(json_pessoa);
            return HTTP_UNPROCESSABLE_ENTITY;
        }
        json_t *json_apelido = json_object_get(json_pessoa, "apelido");
        json_t *json_nome = json_object_get(json_pessoa, "nome");
        json_t *json_stack = json_object_get(json_pessoa, "stack");

        if (string_util_utf8_strlen(json_string_value(json_nome)) <= PESSOA_NOME_LEN && 
            string_util_utf8_strlen(json_string_value(json_apelido)) <= PESSOA_APELIDO_LEN &&
            string_util_utf8_strlen(json_string_value(json_nascimento)) <= PESSOA_NASCIMENTO_LEN &&
            is_stacks_validas(json_stack, PESSOA_EACH_STACK_LEN)) {

            Pessoa *pessoa = malloc(sizeof(Pessoa));
            strzcpy(pessoa->nome, json_string_value(json_nome), UTF8_PESSOA_NOME_LEN);
            strzcpy(pessoa->apelido, json_string_value(json_apelido), UTF8_PESSOA_APELIDO_LEN);
            strzcpy(pessoa->nascimento, json_string_value(json_nascimento), PESSOA_NASCIMENTO_LEN);
            build_string_stacks(pessoa->stack, json_stack, PESSOA_STACKS_LEN);

            if (pessoa_service_save(pessoa)) {
                sprintf(buffer_response_save, response_save, pessoa->id);
                resp = buffer_response_save;
            } else {
                resp = HTTP_UNPROCESSABLE_ENTITY;
            }
            
            free(pessoa);
        }
        json_decref(json_pessoa);
    } else {
        log_error("Request sem body");
    }
    return resp;
}

char response_find_by_id[] = "HTTP/1.1 200 OK""\r\n"                                  
                                  "Content-Type: application/json; charset=utf-8""\r\n"
                                  "Content-Length: %d""\r\n"
                                  "\r\n"
                                  "%s";

char *pessoa_controller_find_by_id(const char *url) {
    char *param = extract_url_param(url);

    if (strlen(param) > 0) {
        Pessoa *pessoa = pessoa_service_find_by_id(param);
        if (pessoa != NULL) {
            json_t *json_pessoa = build_json_pessoa(pessoa);
            char *buffer_pessoa = json_dumps(json_pessoa, 0);
            sprintf(buffer_response_find_by_id, response_find_by_id, strlen(buffer_pessoa), buffer_pessoa);
            free(buffer_pessoa);
            json_decref(json_pessoa);
            free(pessoa);
            return buffer_response_find_by_id;
        } else {
            return HTTP_NOT_FOUND;
        }
    }
    return HTTP_BAD_REQUEST;
}

static char response_find_by_termo[] = "HTTP/1.1 200 OK""\r\n"                                  
                                    "Content-Type: application/json; charset=utf-8""\r\n"
                                    "Content-Length: %d""\r\n"
                                    "\r\n"
                                    "%s";

char *pessoa_controller_find_by_termo(const char *url) {
    char *resp = HTTP_BAD_REQUEST;
    char *param = extract_form_param(url);

    if (param[0] == 't' && param[1] == '=' && param[2] != 0) {
        PessoaList pessoas = pessoa_service_find_by_termo(&param[2]);

        if (pessoas.size > 0) {
            json_t *json_pessoas = json_array();
            json_t *json_pessoa;
            for (int i = 0; i < pessoas.size; i++) {
                json_pessoa = build_json_pessoa(&pessoas.values[i]);
                json_array_append_new(json_pessoas, json_pessoa);
            }
            char *buffer_pessoas = json_dumps(json_pessoas, 0);
            sprintf(buffer_response_find_by_termo, response_find_by_termo, strlen(buffer_pessoas), buffer_pessoas);
            free(buffer_pessoas);
            json_decref(json_pessoas);
            
            resp = buffer_response_find_by_termo;
        } else {
            resp = HTTP_OK;
        }  
        pessoa_service_free_list(pessoas);      
    } 
    return resp;
}

static char response_contagem[] = "HTTP/1.1 200 OK""\r\n"                                  
                                "Content-Type: text/plain; charset=utf-8""\r\n"
                                "Content-Length: %d""\r\n"
                                "\r\n"
                                "%d";

char *pessoa_controller_count(const char *url) {
    uint64_t count = pessoa_service_count();
    char snum[11];
    sprintf(snum, "%"PRId64"", count);
    sprintf(buffer_response_contagem, response_contagem, strlen(snum), count);
    return buffer_response_contagem;
}

//////////////////////////////////////////////////////////////////////////////////////

char *extract_param(char *buffer, const char *url, char delimiter, size_t size) {
    int len = strlen(url);

    int i;
    for (i = len - 1; i >= 0 && url[i] != delimiter; i--);

    if (i >= 0) {
        strzcpy(buffer, &url[i + 1], size + 1);        
    }
    return buffer;
}

char *extract_url_param(const char *url) {
    return extract_param(buffer_url_param, url, '/', MAX_BUFFER_URL_PARAM);
}

char *extract_form_param(const char *url) {
    char *temp = extract_param(buffer_form_param, url, '?', MAX_BUFFER_FORM_PARAM);
    
    for (int i = 0; i < strlen(temp); i++) {
        if (temp[i] == '+') {
            temp[i] = ' ';
        }
    }
    return temp;
}

json_t *build_json_stacks(char *stacks) {
    if (stacks != NULL) {
        json_t *json_stacks = json_array();
        json_t *json_stack;
        int last = 0;
        size_t size = strlen(stacks);
        for (int i = 0; i < size; i++) {
            if (stacks[i] == ';') {
                stacks[i] = 0;
                json_stack = json_string(&stacks[last]);
                json_array_append_new(json_stacks, json_stack);
                stacks[i] = ';';
                last = i + 1;
            }
        }
        return json_stacks;
    }
    return NULL;
}

void build_string_stacks(char *stacks, json_t *json_stacks, size_t size) {
    if (json_is_array(json_stacks)) {
        stacks[0] = 0;
        size_t array_size = json_array_size(json_stacks);
        const char *temp_str;
        size_t temp_len;
        size_t actual_size = 0;
        for (int i = 0; i < array_size; i++) {
            json_t *json_stack = json_array_get(json_stacks, i);
            temp_str = json_string_value(json_stack);
            temp_len = strlen(temp_str);

            if ((actual_size + temp_len + 1) > size) {
                log_warn("Tamanho stacks excedido");
                break;
            }
            strzcpy(&stacks[actual_size], temp_str, temp_len);
            actual_size += temp_len;
            stacks[actual_size] = ';';
            actual_size++;
            stacks[actual_size] = 0;  
        }
    }
}

bool is_stacks_validas(json_t *json_stacks, size_t size) {
    if (json_is_null(json_stacks)) {
        return true;
    }
    if (!json_is_array(json_stacks)) {
        return false;
    }
    size_t array_size = json_array_size(json_stacks);
    for (int i = 0; i < array_size; i++) {
        json_t *json_stack = json_array_get(json_stacks, i);
        if (string_util_utf8_strlen(json_string_value(json_stack)) > size) {
            return false;
        }            
    }
    return true;  
}

json_t *build_json_pessoa(Pessoa *pessoa) {
    return json_pack("{s:s?, s:s?, s:s?, s:s?, s:o?}", 
        "id", pessoa->id, 
        "apelido", pessoa->apelido, 
        "nome", pessoa->nome, 
        "nascimento", pessoa->nascimento,
        "stack", build_json_stacks(pessoa->stack)
        );
}

bool is_data_valida(const char *data) {
    struct tm   time_val;
    char *res = strptime(data, "%Y-%m-%d", &time_val);
    
    if (res != NULL && *res == 0) {
        if (time_val.tm_mon == 1) {
            if ((time_val.tm_year % 4) == 0) {
                return time_val.tm_mday <= 29;
            } else {
                return time_val.tm_mday <= 28;
            }
        }
        return true;
    }
    return false;
}

