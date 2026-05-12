cmake_minimum_required(VERSION 3.15)
project(mungo_example_http)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        BS_thread_pool
        GIT_REPOSITORY  https://github.com/bshoshany/thread-pool.git
        GIT_TAG         v5.1.0
)
FetchContent_MakeAvailable(bs_thread_pool)
add_library(BS_thread_pool INTERFACE)
target_include_directories(BS_thread_pool INTERFACE "${bs_thread_pool_SOURCE_DIR}/include")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE mungo BS_thread_pool)