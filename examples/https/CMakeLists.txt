cmake_minimum_required(VERSION 3.15)
project(mungo_example_https)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE mungo BS_thread_pool)

find_program(OPENSSL_BIN openssl)
if (OPENSSL_BIN)
    if(WIN32)
        set(TLS_SCRIPT "${CMAKE_SOURCE_DIR}/build/_deps/mgxx-src/examples/tls.cmd")
    else()
        set(TLS_SCRIPT "${CMAKE_SOURCE_DIR}/build_linux/_deps/mgxx-src/examples/tls.sh")
    endif()

    add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND "${TLS_SCRIPT}" "${OPENSSL_BIN}" "${CMAKE_SOURCE_DIR}/build"
            COMMENT "Generating self-signed certificates..."
    )
else ()
    message(WARNING "OpenSSL not found. Please generate certificates manually.")
endif ()
