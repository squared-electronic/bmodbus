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

arduino_project_add(
        arduino_client_example
        "${CMAKE_CURRENT_SOURCE_DIR}/arduino_client_example"
        "adafruit:samd:adafruit_feather_m4"
)

arduino_project_add(
        arduino_client_example
        "${CMAKE_CURRENT_SOURCE_DIR}/arduino_client_example"
        "arduino:avr:uno"
)
