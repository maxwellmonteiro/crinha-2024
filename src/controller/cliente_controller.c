#include "cliente_controller.h"
#include "../service/cliente_service.h"
#include "../service/transacao_service.h"
#include "../server/router.h"
#include "../server/router.h"
#include "../util/string_util.h"
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>

#include <jansson.h>
#include "../util/log.h"

#define MAX_BUFFER_URL_PARAM 128
#define MAX_BUFFER_FIND_BY_ID 1024
#define MAX_BUFFER_TRANSACAO_SAVE 512

#define MATCH_ONE_URL_PARAM(STR, STR2) ("^\\"STR"\\/([a-zA-Z0-9\\-]*)\\"STR2"$")
#define EXTRACT_ONE_URL_PARAM "\\/([0-9]*)\\/"

static regex_t matcher_save;
static regex_t matcher_find_by_id;
static regex_t matcher_find_by_termo;
static regex_t matcher_count;

static Route route_find_by_id = { HTTP_GET, MATCH_ONE_URL_PARAM("/clientes", "/extrato"), cliente_controller_find_by_id, NULL };
static Route route_transacao_save = { HTTP_POST, MATCH_ONE_URL_PARAM("/clientes", "/transacoes"), NULL, cliente_controller_save_transacao };

static char buffer_url_param[MAX_BUFFER_URL_PARAM];
static char buffer_response_find_by_id[MAX_BUFFER_FIND_BY_ID];
static char buffer_response_trasacao_save[MAX_BUFFER_TRANSACAO_SAVE];

char *extract_param(char *buffer, const char *url, char delimiter, size_t size);
char *extract_url_param(const char *url);
json_t *build_json_saldo(Cliente *cliente);
json_t *build_json_saldo_data(Cliente *cliente);
json_t *build_json_transacoes(TransacaoList transacoes);
json_t *build_json_transacao(Transacao *transacao);
json_t *build_json_extrato(json_t *json_saldo, json_t *json_transacoes);

bool is_id_cliente_valido(const char *id);
bool is_descricao_transacao_valida(const char *descricao);
bool is_tipo_transacao_valido(const char *tipo);
bool is_valor_transacao_valido(const char *valor);
bool digits_only(const char *s);

void cliente_controller_init() {  
    router_add_route(&route_find_by_id);
    router_add_route(&route_transacao_save);
}

char response_save_transacao[] = "HTTP/1.1 200 OK""\r\n"
                                "Location: /clientes/%s/transacoes/""\r\n"                            
                                "Content-Length: %d""\r\n"
                                "\r\n"
                                "%s";


char response_find_by_id[] = "HTTP/1.1 200 OK""\r\n"                                  
                                "Content-Type: application/json; charset=utf-8""\r\n"
                                "Content-Length: %d""\r\n"
                                "\r\n"
                                "%s";                            
                            

char *cliente_controller_save_transacao(const char *url, const char *body) {
    char *resp = HTTP_BAD_REQUEST;
    char *param = extract_url_param(url);
    if (is_id_cliente_valido(param) && body != NULL) {
        int id_cliente = atoi(param);
        json_t *json_transacao = json_loads(body, 0, NULL);

        json_t *json_valor = json_object_get(json_transacao, "valor");
        json_t *json_tipo = json_object_get(json_transacao, "tipo");
        json_t *json_descricao = json_object_get(json_transacao, "descricao");

        if (is_descricao_transacao_valida(json_string_value(json_descricao)) && 
            is_tipo_transacao_valido(json_string_value(json_tipo)) &&
            is_valor_transacao_valido(json_string_value(json_valor))
            ) {

            Transacao *transacao = malloc(sizeof(Transacao));
            transacao->id_cliente = id_cliente;
            transacao->valor = json_integer_value(json_valor);
            transacao->tipo = json_string_value(json_tipo)[0];
            strzcpy(transacao->descricao, json_string_value(json_descricao), UTF8_TRANSACAO_DESCRICAO_LEN);

            if (transacao_service_save(transacao)) {
                Cliente *cliente = cliente_service_find_one(id_cliente);
                if (cliente != NULL) {
                    json_t *json_saldo = build_json_saldo(cliente);
                    char *buffer_saldo = json_dumps(json_saldo, 0);
                    sprintf(buffer_response_trasacao_save, response_save_transacao, transacao->id, strlen(buffer_saldo), buffer_saldo);
                    resp = buffer_response_trasacao_save;
                    json_decref(json_saldo);
                    free(buffer_saldo);
                    free(cliente);
                } else {
                    resp = HTTP_UNPROCESSABLE_ENTITY;
                }
            } else {
                resp = HTTP_UNPROCESSABLE_ENTITY;
            }
            free(transacao);
        }
        json_decref(json_transacao);
    } else {
        log_error("Request sem body");
    }
    return resp;
}

char *cliente_controller_find_by_id(const char *url) {
    char *param = extract_url_param(url);

    if (is_id_cliente_valido(param)) {
        int id_cliente = atoi(param);
        Cliente *cliente = cliente_service_find_one(id_cliente);
        if (cliente != NULL) {
            TransacaoList transacoes = transacao_service_find_last_10(id_cliente);
            json_t *json_saldo = build_json_saldo_data(cliente);
            json_t *json_transacoes = build_json_transacoes(transacoes);
            json_t *json_extrato = build_json_extrato(json_saldo, json_transacoes);
            char *buffer_extrato = json_dumps(json_extrato, 0);
            sprintf(buffer_response_find_by_id, response_find_by_id, strlen(buffer_extrato), buffer_extrato);
            free(buffer_extrato);
            json_decref(json_extrato);
            transacao_service_free_list(transacoes);
            free(cliente);
            return buffer_response_find_by_id;
        } else {
            return HTTP_NOT_FOUND;
        }
    }
    return HTTP_BAD_REQUEST;
}

//////////////////////////////////////////////////////////////////////////////////////

bool is_id_cliente_valido(const char *id) {
    return strlen(id) > 0 && strlen(id) < 10 && digits_only(id);
}

bool is_descricao_transacao_valida(const char *descricao) {
    return string_util_utf8_strlen(descricao) <= TRANSACAO_DESCRICAO_LEN;
}

bool is_tipo_transacao_valido(const char *tipo) {
    return tipo[0] == 'c' || tipo[0] == 'd';
}

bool is_valor_transacao_valido(const char *valor) {
    return digits_only(valor) && atol(valor) > 0;
}

bool digits_only(const char *s) {
    while (*s) {
        if (isdigit(*s++) == 0) {
            return false;
        }
    }
    return true;
}

size_t extract_param_regex(char *buffer, const char *pattern, const char *str) {
    int status;
    regex_t regex;
    regmatch_t regmatch;

    status = regcomp(&regex, pattern, REG_EXTENDED);
    if (status) {
        log_error("Falha ao compilar regex (%s)", strerror(errno));
        return false;
    }
    
    status = regexec(&regex, str, 1, &regmatch, 0);
    regfree(&regex);    
    if (status == 0) {
        uint32_t len = regmatch.rm_eo - regmatch.rm_so;
        sprintf(buffer, "%.*s", len - 2, &str[regmatch.rm_so + 1]);
        return len;
    }
    return 0;
}

char *extract_param(char *buffer, const char *url, char delimiter, size_t size) {
    size_t len = extract_param_regex(buffer, EXTRACT_ONE_URL_PARAM, url);
    if (len == 0) {
        buffer[0] = 0;
    }
    return buffer;
}

char *extract_url_param(const char *url) {
    return extract_param(buffer_url_param, url, '/', MAX_BUFFER_URL_PARAM);
}

json_t *build_json_saldo(Cliente *cliente) {
    return json_pack("{s:s?, s:s?}",        
        "limite", cliente->limite,
        "saldo", cliente->saldo
        );
}

json_t *build_json_saldo_data(Cliente *cliente) {
    return json_pack("{s:i, s:s?, s:i}",
        "total", cliente->saldo,
        "data_extrato", NULL,
        "limite", cliente->limite
        );
}

json_t *build_json_transacoes(TransacaoList transacoes) {
    json_t *json_transacoes = json_array();
    json_t *json_transacao;
    size_t size = transacoes.size;
    for (int i = 0; i < size; i++) {
        json_transacao = build_json_transacao(&transacoes.values[i]);
        json_array_append_new(json_transacoes, json_transacao);
    }
    return json_transacoes;
}

json_t *build_json_transacao(Transacao *transacao) {
    return json_pack("{s:s?, s:s?, s:s?, s:s?}",
        "valor", transacao->valor,
        "tipo", transacao->tipo,
        "descricao", transacao->descricao,
        "realizada_em", transacao->realizada_em
        );
}

json_t *build_json_extrato(json_t *json_saldo, json_t *json_transacoes) {
    return json_pack("{s:o?, s:o?}",
        "saldo", json_saldo,
        "ultimas_transacoes", json_transacoes
        );
}


