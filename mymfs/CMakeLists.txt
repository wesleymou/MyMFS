    cmake_minimum_required(VERSION 3.13.2)
    project(mymfs)

    set(CMAKE_CXX_STANDARD 17)
#    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    file(GLOB MAIN_FILE "*.cpp")

    include_directories(lib)
    add_subdirectory(lib)
    add_subdirectory(Google_tests)

    add_executable(mymfs ${MAIN_FILE})

    target_link_libraries(mymfs Mymfs_lib)