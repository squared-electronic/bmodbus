cmake_minimum_required(VERSION 3.25)
project(bmodbus C)

set(CMAKE_C_STANDARD 99)

add_executable(bmodbus main.c)

add_dependencies(bmodbus basic_client)

#Basic client
#Add path to the header files
include_directories(.)
include_directories(examples/basic_client)
add_executable(basic_client bmodbus.c bmodbus.h examples/basic_client/basic_client.c examples/basic_client/basic_client.h)
