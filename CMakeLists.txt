cmake_minimum_required(VERSION 3.10)
project(grpc_lab CXX)

set(CMAKE_CXX_STANDARD 17)

# Encontra os pacotes necessários
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)

# Encontra os executáveis
find_program(PROTOC protoc REQUIRED)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

# Define os arquivos de saída
set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROTO_NAME "notas")
set(PROTO_FILE "${PROTO_PATH}/${PROTO_NAME}.proto")

set(GENERATED_PROTOBUF_PATH "${CMAKE_CURRENT_BINARY_DIR}")
set(PROTO_SRCS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.pb.cc")
set(PROTO_HDRS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.pb.h")
set(GRPC_SRCS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.grpc.pb.cc")
set(GRPC_HDRS "${GENERATED_PROTOBUF_PATH}/${PROTO_NAME}.grpc.pb.h")

# Comando para gerar os arquivos
add_custom_command(
    OUTPUT ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
    COMMAND ${PROTOC}
    ARGS --grpc_out=${GENERATED_PROTOBUF_PATH}
         --cpp_out=${GENERATED_PROTOBUF_PATH}
         -I${PROTO_PATH}
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating gRPC files"
)

# Inclui o diretório de arquivos gerados
include_directories(${GENERATED_PROTOBUF_PATH})

# Cria executáveis
add_executable(servidor_cpp servidor.cpp ${PROTO_SRCS} ${GRPC_SRCS})
add_executable(cliente_cpp cliente.cpp ${PROTO_SRCS} ${GRPC_SRCS})

# Link libraries
target_link_libraries(servidor_cpp ${GRPC_LIBRARIES} ${Protobuf_LIBRARIES})
target_link_libraries(cliente_cpp ${GRPC_LIBRARIES} ${Protobuf_LIBRARIES})