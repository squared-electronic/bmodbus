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
file(COPY_FILE ../client/test_bmodbus_client.c arduino_unit_tests/test_bmodbus_client.h ONLY_IF_DIFFERENT)
file(COPY_FILE ../unity/unity.c arduino_unit_tests/unity.c ONLY_IF_DIFFERENT)
file(COPY_FILE ../unity/unity.h arduino_unit_tests/unity.h ONLY_IF_DIFFERENT)
file(COPY_FILE ../unity/unity_internals.h arduino_unit_tests/unity_internals.h ONLY_IF_DIFFERENT)
file(COPY_FILE ../../bmodbus.c arduino_unit_tests/bmodbus.c ONLY_IF_DIFFERENT)
file(COPY_FILE ../../bmodbus.h arduino_unit_tests/bmodbus.h ONLY_IF_DIFFERENT)
arduino_project_add(
        arduino_unit_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/arduino_unit_tests"
        "adafruit:samd:adafruit_feather_m4"
        DEPENDENCIES "arduino_unit_tests/test_bmodbus_client.h" "arduino_unit_tests/unity.c" "arduino_unit_tests/unity.h" "arduino_unit_tests/unity_internals.h" "arduino_unit_tests/bmodbus.c" "arduino_unit_tests/bmodbus.h"
)

arduino_project_add(
        arduino_unit_tests
        "${CMAKE_CURRENT_SOURCE_DIR}/arduino_unit_tests"
        "esp32:esp32:esp32"
        DEPENDENCIES "arduino_unit_tests/test_bmodbus_client.h" "arduino_unit_tests/unity.c" "arduino_unit_tests/unity.h" "arduino_unit_tests/unity_internals.h" "arduino_unit_tests/bmodbus.c" "arduino_unit_tests/bmodbus.h"
)