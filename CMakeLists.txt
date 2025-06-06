cmake_minimum_required(VERSION 3.29.3 FATAL_ERROR)


project(bohares LANGUAGES C)

set(BOHARES_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/source)

file(GLOB_RECURSE BOHARES_SRC_FILES CONFIGURE_DEPENDS 
    ${BOHARES_SRC_DIR}/*.h
    ${BOHARES_SRC_DIR}/*.c)


add_executable(${PROJECT_NAME} ${BOHARES_SRC_FILES})


if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments 
        -Wno-gnu-anonymous-struct -Wno-nested-anon-types)
endif()


target_precompile_headers(${PROJECT_NAME} PRIVATE ${BOHARES_SRC_DIR}/pch.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${BOHARES_SRC_DIR})