cmake_minimum_required(VERSION 2.8.4)
project(faops C)

set(CMAKE_C_COMPILER gcc)

find_package(ZLIB REQUIRED)

set(SOURCE_FILES
    faops.c
    khash.h
    kseq.h)

include_directories(${ZLIB_INCLUDE_DIRS})

add_executable(faops ${SOURCE_FILES})
target_link_libraries(faops ${ZLIB_LIBRARIES})
