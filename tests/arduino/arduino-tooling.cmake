# In a separate file, e.g., ArduinoUtils.cmake, then include it:
# include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ArduinoUtils.cmake)
# Or directly in your main CMakeLists.txt if preferred.
find_program(ARDUINO_CLI_EXECUTABLE NAMES arduino-cli
        DOC "Path to the Arduino CLI executable")

if(NOT ARDUINO_CLI_EXECUTABLE)
    message(FATAL_ERROR "Arduino CLI not found. Please ensure it's in your PATH or specify its location.")
endif()

function(arduino_project_add PROJECT_NAME SKETCH_DIR BOARD_FQBN)
    # Optional: Define a default build path if not provided
    # set(DEFAULT_BUILD_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_build")

    # Arguments:
    # PROJECT_NAME: A unique name for this Arduino project (e.g., "my_esp32_sensor", "uno_blinky")
    # SKETCH_DIR: The absolute path to the Arduino sketch folder (e.g., "${CMAKE_CURRENT_SOURCE_DIR}/sketches/my_blinky")
    # BOARD_FQBN: The Fully Qualified Board Name (e.g., "arduino:avr:uno", "esp32:esp32:esp32")
    # OUTPUT_FILE_NAME: The name and full path of the output file (e.g., "build/arduino.avr.uno/arduino_client_example.ino.hex")

    # Define build target
    ###--build-path "${DEFAULT_BUILD_PATH}"
    # set(TEMP_TARGET_NAME ${OUTPUT_FILE_NAME})
    set(TEMP_TARGET_NAME "build/${BOARD_FQBN}/${PROJECT_NAME}.hex")
    string(REPLACE "/" "." TEMP_TARGET_NAME ${TEMP_TARGET_NAME})
    string(REPLACE ":" "." TEMP_TARGET_NAME ${TEMP_TARGET_NAME})
    add_custom_target(
            ${TEMP_TARGET_NAME} ALL
            COMMAND ${ARDUINO_CLI_EXECUTABLE} compile
            --fqbn "${BOARD_FQBN}"
            "${SKETCH_DIR}"
            BYPRODUCTS ${OUTPUT_FILE_NAME}
            COMMENT "Building Arduino project '${PROJECT_NAME}' for board '${BOARD_FQBN}'"
            VERBATIM
    )

    # Define upload target (optional, only create if ARDUINO_CLI_EXECUTABLE is found)
    # Note: Upload requires a port. We can make this configurable.
#    if(ARDUINO_CLI_EXECUTABLE)
#        # Add a configurable variable for the serial port
#        # This can be set via -DARDUINO_PORT=/dev/ttyUSB0 when configuring CMake
#        option(ARDUINO_PORT_FOR_${PROJECT_NAME} "" "Serial port for ${PROJECT_NAME}")
#        # If the option is empty, user needs to set it
#        if(NOT ARDUINO_PORT_FOR_${PROJECT_NAME})
#            message(STATUS "No ARDUINO_PORT specified for '${PROJECT_NAME}'. Skipping upload target.")
#        else()
#            add_custom_target(
#                    ${PROJECT_NAME}_upload
#                    COMMAND ${ARDUINO_CLI_EXECUTABLE} upload
#                    --fqbn "${BOARD_FQBN}"
#                    --port "${ARDUINO_PORT_FOR_${PROJECT_NAME}}"
#                    --build-path "${DEFAULT_BUILD_PATH}"
#                    "${SKETCH_DIR}"
#                    COMMENT "Uploading Arduino project '${PROJECT_NAME}' to port '${ARDUINO_PORT_FOR_${PROJECT_NAME}}'"
#                    VERBATIM
#                    DEPENDS ${PROJECT_NAME}_build # Ensure it's built before uploading
#            )
#        endif()
#    endif()

    # You could also add other targets here, e.g., `monitor`, `lib install`, etc.
    # For example, to install libraries needed by this project:
    # add_custom_target(
    #     ${PROJECT_NAME}_install_libs
    #     COMMAND ${ARDUINO_CLI_EXECUTABLE} lib install "MyAwesomeLibrary"
    #     COMMAND ${ARDUINO_CLI_EXECUTABLE} lib install "AnotherLib@1.2.3"
    #     COMMENT "Installing libraries for ${PROJECT_NAME}"
    # )
    # add_dependencies(${PROJECT_NAME}_build ${PROJECT_NAME}_install_libs) # Make build depend on lib install
endfunction()