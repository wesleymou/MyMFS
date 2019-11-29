cmake_minimum_required(VERSION 3.13.2)
project(Mymfs_lib)

set(CMAKE_CXX_STANDARD 17)

file(GLOB HEADER_FILES "*.h")

file(GLOB SOURCE_FILES "*.cpp")

add_library(Mymfs_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(Mymfs_lib stdc++fs)
target_link_libraries(Mymfs_lib -lz)
target_link_libraries(Mymfs_lib -pthread)