include(FetchContent)
FetchContent_Declare(
        BS_thread_pool
        GIT_REPOSITORY  https://github.com/bshoshany/thread-pool.git
        GIT_TAG         v5.1.0
)
FetchContent_MakeAvailable(bs_thread_pool)
add_library(BS_thread_pool INTERFACE)
target_include_directories(BS_thread_pool INTERFACE "${bs_thread_pool_SOURCE_DIR}/include")

add_subdirectory(http/)
add_subdirectory(https/)
add_subdirectory(router/)