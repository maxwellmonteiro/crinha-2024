cmake_minimum_required(VERSION 3.15)
project(rinha C)

find_package(PostgreSQL REQUIRED)
find_package(libuuid REQUIRED)
find_package(llhttp REQUIRED)
find_package(jansson REQUIRED)

add_executable(${PROJECT_NAME} 
  src/main.c 
  src/util/db_connection.c 
  src/service/cliente_service.c
  src/service/transacao_service.c 
  src/repository/cliente_repository.c 
  src/repository/transacao_repository.c 
  src/util/uuid_util.c
  src/util/string_util.c
  src/util/log.c
  src/util/array_list.c
  src/util/env_util.c
  src/server/server.c
  src/server/http_parser.c
  src/server/router.c
  src/controller/cliente_controller.c
  )

target_link_libraries(${PROJECT_NAME} PostgreSQL::PostgreSQL)
target_link_libraries(${PROJECT_NAME} libuuid::libuuid)
target_link_libraries(${PROJECT_NAME} llhttp::llhttp)
target_link_libraries(${PROJECT_NAME} jansson::jansson)
