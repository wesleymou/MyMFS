    cmake_minimum_required(VERSION 3.13.2)
    project(mymfs)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(SOURCE_FILES mymfs.cpp comandos.cpp comandos.h)

    add_executable(mymfs ${SOURCE_FILES})

    target_link_libraries(mymfs stdc++fs)
    target_link_libraries(mymfs -lz)
    target_link_libraries(mymfs -pthread)