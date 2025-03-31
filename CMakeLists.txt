cmake_minimum_required(VERSION 3.25)
project(bmodbus C)

set(CMAKE_C_STANDARD 11)

add_executable(bmodbus main.c)

add_dependencies(bmodbus basic_client)

#Basic client
#Add path to the header files
include_directories(.)
#include_directories()
add_executable(basic_client bmodbus.c bmodbus.h examples/basic_client/basic_client.c examples/basic_client/basic_client.h)

#Unit tests
target_include_directories(basic_client PRIVATE examples/basic_client)


add_executable(unit_testing tests/unity/unity.c tests/client/test_bmodbus_client.c bmodbus.c)
target_include_directories(unit_testing PRIVATE tests/client)
target_include_directories(unit_testing PRIVATE tests/unity)
target_compile_definitions(unit_testing PRIVATE -DUNIT_TESTING)
target_compile_options(unit_testing PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME unit_testing COMMAND unit_testing)
enable_testing()
# HEre we force the unit_testing target to be built
#add_custom_target(run_tests COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure DEPENDS unit_testing)
#add_dependencies(basic_client run_tests)