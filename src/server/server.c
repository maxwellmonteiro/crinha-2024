#include "server.h"
#include "http_parser.h"
#include "router.h"
#include "../util/log.h"
#include "../util/array_list.h"
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>


#define SOCKET_MAX_READ_BUFFER 1024
#define SOCKET_MAX_QUEUED_CONN 100
#define SOCKET_MAX_READ_RETRIES 3
#define SOCKET_MAX_READ_WAIT 1000

int socket_create() {
    int opt = 1;
    int sock_handle = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_handle < 0) {
        log_fatal("Falha ao criar o socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sock_handle, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        log_fatal("Falha ao definir socket options (%s)", strerror(errno));
        shutdown(sock_handle, SHUT_RDWR);
        exit(EXIT_FAILURE);
    }
    return sock_handle;
}

void socket_listen(int sock_handle, struct sockaddr_in *address) {
    if (bind(sock_handle, (struct sockaddr *) address, sizeof(*address)) < 0) {
        log_fatal("Falha ao fazer bind (%s)", strerror(errno));
        shutdown(sock_handle, SHUT_RDWR);
        exit(EXIT_FAILURE);
    }
    if (listen(sock_handle, SOCKET_MAX_QUEUED_CONN) < 0) {
        log_fatal("Falha ao fazer listen (%s)", strerror(errno));
        shutdown(sock_handle, SHUT_RDWR);
        exit(EXIT_FAILURE);
    }
}

void socket_loop(int sock_handle, struct sockaddr_in *address, llhttp_t *parser) {
    int new_socket;
    int addrlen = sizeof(*address);
    char buffer[SOCKET_MAX_READ_BUFFER] = { 0 };

    while (true) {
        if ((new_socket = accept(sock_handle, (struct sockaddr *) address, (socklen_t *) &addrlen)) >= 0) {
            int valread;
            char* response;

            valread = recv(new_socket, buffer, SOCKET_MAX_READ_BUFFER, 0);
            while (valread > 0) {
                buffer[valread] = 0;
                if (http_parser_parse(parser, buffer, valread) && http_parser_request_is_complete(parser)) {
                    uint8_t method = parser->method;
                    char *url = ((HttpRequest *) parser->data)->url;
                    char *body = ((HttpRequest *) parser->data)->body;  
                    response = router_route(method, url, body);
                    send(new_socket, response, strlen(response), 0);
                    http_parser_reset_request(parser);
                } else {
                    log_error("Falha no parse do request http (%s)", buffer);
                }
                valread = recv(new_socket, buffer, SOCKET_MAX_READ_BUFFER, MSG_DONTWAIT);
            }
            shutdown(new_socket, SHUT_RDWR);
            close(new_socket);
        } else {
            log_error("Falha ao fazer accept (%s)", strerror(errno));
            sleep(1);
        }
    }
}

void server_init(uint16_t port) {
    log_info("Iniciando servidor ...");
    int sock_handle = socket_create();

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    socket_listen(sock_handle, &address);
    
    llhttp_t *parser = http_parser_init();

    char ipv4[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &address.sin_addr, ipv4, INET_ADDRSTRLEN);
    log_info("Servidor iniciado em %s:%d", ipv4, port);

    socket_loop(sock_handle, &address, parser);

    http_parser_close(parser);

    shutdown(sock_handle, SHUT_RDWR);    
}