cmake_minimum_required(VERSION 3.15)
project(mymfs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(mymfs
        mymfs.cpp
        comandos.h)
target_link_libraries(mymfs stdc++fs)
