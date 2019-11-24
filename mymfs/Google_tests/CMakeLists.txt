project(Google_tests)

add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(Config_Tests variaveis.h config.cpp)
target_link_libraries(Config_Tests Mymfs_lib)
target_link_libraries(Config_Tests gtest gtest_main)

add_executable(Import_Tests variaveis.h import.cpp)
target_link_libraries(Import_Tests Mymfs_lib )
target_link_libraries(Import_Tests gtest gtest_main)

add_executable(Comparacoes_Tests variaveis.h comparacoes.cpp)
target_link_libraries(Comparacoes_Tests Mymfs_lib)
target_link_libraries(Comparacoes_Tests gtest gtest_main)
