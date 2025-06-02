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
set(MY_ARDUINO_TARGETS
        "adafruit:samd:adafruit_feather_m4"
        "esp32:esp32:esp32"
        "arduino:avr:uno"
)

# arduino_client_example
file(COPY_FILE ../../bmodbus.c arduino_client_example/bmodbus.c ONLY_IF_DIFFERENT)
file(COPY_FILE ../../bmodbus.h arduino_client_example/bmodbus.h ONLY_IF_DIFFERENT)
foreach(BOARD_FQBN ${MY_ARDUINO_TARGETS})
    message(STATUS "Adding unit tests for board: ${BOARD_FQBN}")
    arduino_project_add(
            arduino_unit_tests
            "${CMAKE_CURRENT_SOURCE_DIR}/arduino_unit_tests"
            "${BOARD_FQBN}"
            DEPENDENCIES "arduino_unit_tests/test_bmodbus_client.h" "arduino_unit_tests/unity.c" "arduino_unit_tests/unity.h" "arduino_unit_tests/unity_internals.h" "arduino_unit_tests/bmodbus.c" "arduino_unit_tests/bmodbus.h"
    )
    arduino_project_add(
            arduino_client_example
            "${CMAKE_CURRENT_SOURCE_DIR}/arduino_client_example"
            "${BOARD_FQBN}"
            DEPENDENCIES "arduino_client_example/bmodbus.c" "arduino_client_example/bmodbus.h" "arduino_client_example/arduino_client_example.ino"
    )
endforeach()
