cmake_minimum_required(VERSION 3.15)
project(mungo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        mgxx
        GIT_REPOSITORY https://github.com/poirierlouis/mgxx.git
        GIT_TAG        main
)
FetchContent_MakeAvailable(mgxx)

add_library(${PROJECT_NAME} STATIC
        src/app.cpp
        src/filesystem.cpp
        src/request.cpp
        src/response.cpp
        src/route.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PUBLIC mgxx::mgxx)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_EXCEPTIONS=0)
    target_compile_options(${PROJECT_NAME} PRIVATE /GR- /EHs- /EHc- /Zc:throwingNew-)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -fno-exceptions -fno-rtti)
endif ()

option(MUNGO_BUILD_EXAMPLES "Build examples" ON)
if (MUNGO_BUILD_EXAMPLES)
    message(STATUS "Configuring examples...")
    add_subdirectory(examples)
endif ()