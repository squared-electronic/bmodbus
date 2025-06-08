cmake_minimum_required(VERSION 3.16)

#add_executable(arduino_avr arduino_client_example.ino ../bmodbus.c ../bmodbus.h)
# add_executable(build/arduino.avr.uno/arduino_client_example.ino.hex)
# arduino-cli compile --fqbn adafruit:samd:adafruit_itsybitsy_m4  --export-binaries .\arduino_client_example.ino
#add_custom_command(
#        TARGET build/arduino.avr.uno/arduino_client_example.ino.hex
#        POST_BUILD
#        COMMAND arduino-cli compile --fqbn arduino:avr:uno  --export-binaries arduino_client_example.ino
#)

#arduino-cli board listall | grep avr

include(arduino-tooling.cmake)

#arduino_project_add(
#        arduino_client_example
#        "${CMAKE_CURRENT_SOURCE_DIR}/arduino_client_example"
#        "adafruit:samd:adafruit_feather_m4"
#)
#
#arduino_project_add(
#        arduino_client_example
#        "${CMAKE_CURRENT_SOURCE_DIR}/arduino_client_example"
#        "arduino:avr:uno"
#)
#add_custom_command(
#        TARGET
#)

# unit tests
file(COPY_FILE ../client/test_bmodbus_client.c arduino_unit_tests/test_bmodbus_client.h ONLY_IF_DIFFERENT)
file(COPY_FILE ../unity/unity.c arduino_unit_tests/unity.c ONLY_IF_DIFFERENT)
file(COPY_FILE ../unity/unity.h arduino_unit_tests/unity.h ONLY_IF_DIFFERENT)
file(COPY_FILE ../unity/unity_internals.h arduino_unit_tests/unity_internals.h ONLY_IF_DIFFERENT)
file(COPY_FILE ../../bmodbus.c arduino_unit_tests/bmodbus.c ONLY_IF_DIFFERENT)
file(COPY_FILE ../../bmodbus.h arduino_unit_tests/bmodbus.h ONLY_IF_DIFFERENT)
# arduino_client_example
file(COPY_FILE ../../bmodbus.c arduino_client_example/bmodbus.c ONLY_IF_DIFFERENT)
file(COPY_FILE ../../bmodbus.h arduino_client_example/bmodbus.h ONLY_IF_DIFFERENT)

set(arduino_build_configurations
        "adafruit:samd:adafruit_feather_m4 "
        "arduino:avr:uno -DTEST_SKIP_CLIENT_ONLY_TESTS"
        "esp32:esp32:esp32 "
        "esp32:esp32:esp32s3 "
        "rp2040:rp2040:rpipico "
        "rp2040:rp2040:rpipico2w "
        "rp2040:rp2040:rpipico2w:arch=riscv "
)

foreach(BOARD_CONFIG IN LISTS arduino_build_configurations)
    MESSAGE(STATUS "Processing board configuration: ${BOARD_CONFIG}")
    string(REPLACE " " ";" temp_list_for_splitting "${BOARD_CONFIG}")
    list(GET temp_list_for_splitting 0 BOARD_FQBN)
    MESSAGE(STATUS "Board FQBN: ${BOARD_FQBN}")
    list(LENGTH temp_list_for_splitting list_length)
    if(list_length EQUAL 2)
        list(GET temp_list_for_splitting 1 EXTRA_DEFINITIONS)
    else()
        set(EXTRA_DEFINITIONS "")
    endif()
    MESSAGE(STATUS "Extra definitions: ${EXTRA_DEFINITIONS}")
    message(STATUS "Adding unit tests for board: ${BOARD_FQBN}")
    arduino_project_add(
            arduino_unit_tests
            "${CMAKE_CURRENT_SOURCE_DIR}/arduino_unit_tests"
            "${BOARD_FQBN}"
            "${EXTRA_DEFINITIONS}"
            DEPENDENCIES "arduino_unit_tests/test_bmodbus_client.h" "arduino_unit_tests/unity.c" "arduino_unit_tests/unity.h" "arduino_unit_tests/unity_internals.h" "arduino_unit_tests/bmodbus.c" "arduino_unit_tests/bmodbus.h"
    )
    arduino_project_add(
            arduino_client_example
            "${CMAKE_CURRENT_SOURCE_DIR}/arduino_client_example"
            "${BOARD_FQBN}"
            "${EXTRA_DEFINITIONS}"
            DEPENDENCIES "arduino_client_example/bmodbus.c" "arduino_client_example/bmodbus.h" "arduino_client_example/arduino_client_example.ino"
    )
endforeach()

#Add the 8051 build
arduino_project_add(
        8051_client
        "${CMAKE_CURRENT_SOURCE_DIR}/8051_client"
        "CH55xDuino:mcs51:ch552:clock=16internal"
        "-DBMB_MAXIMUM_MESSAGE_SIZE=16 -DBMODBUS_NO_MASTER"
        DEPENDENCIES "8051_client/bmodbus.c" "8051_client/bmodbus.h" "8051_client/8051_client.ino"
)