//
// Created by billm on 3/24/2025.
//

#include <stdint.h>
#include <stdio.h>

#include "bmodbus.h"
#include "bmodbus_internals.h"
#include "unity.h"

void setUp(void) {
    // Set up code before each test
}

void tearDown(void) {
    // Clean up code after each test
}

#define byte_timing_microseconds(BAUDRATE) (1000000 / ((BAUDRATE)/10))

void test_client_simple_write(void) {
    uint8_t writing_registers_address_0x0708_at_slave_2[] = {0x02, 0x10, 0x07, 0x08, 0x00, 0x02, 0x04, 0x02, 0x03, 0x04, 0x05, 0xe8, 0x06, };
    modbus_uart_response_t * response = NULL;
    modbus1_init(2);
    uint32_t fake_time = 0;
    fake_time += byte_timing_microseconds(38400) * 1; // 1 byte
    for(uint16_t i=0;i<sizeof(writing_registers_address_0x0708_at_slave_2);i++) {
        //Ensure we are not giving a response
        TEST_ASSERT_EQUAL_MESSAGE(NULL, modbus1_get_request(), "modbus request is ready too early");
        modbus1_next_byte(fake_time, writing_registers_address_0x0708_at_slave_2[i]);
        fake_time += byte_timing_microseconds(38400) * 1; // 1 byte
    }
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, modbus1_get_request(), "modbus request is ready too early");
    TEST_ASSERT_EQUAL_MESSAGE(CLIENT_STATE_PROCESSING_REQUEST, modbus1_testing->state, "modbus1_testing->state");
    TEST_ASSERT_EQUAL_MESSAGE(0x10, modbus1_testing->payload.request.function, "modbus1_testing->payload.request.function");
    TEST_ASSERT_EQUAL_MESSAGE(0x0708, modbus1_testing->payload.request.address, "modbus1_testing->payload.request.address");
    TEST_ASSERT_EQUAL_MESSAGE(2, modbus1_testing->payload.request.size, "modbus1_testing->payload.request.size");
    TEST_ASSERT_EQUAL_MESSAGE(0x0203, modbus1_testing->payload.request.data[0], "modbus1_testing->payload.request.data[0]");
    TEST_ASSERT_EQUAL_MESSAGE(0x0405, modbus1_testing->payload.request.data[1], "modbus1_testing->payload.request.data[1]");

    //Here we say it was successful, and continue on
    response = modbus1_get_response();
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, response, "modbus response is not ready");
    TEST_ASSERT_EQUAL_MESSAGE(8, response->size, "response->size");
}

void test_client_single_write(void){
    uint8_t writing_register_address_0x0708_at_slave_2[] = {0x02, 0x06, 0x07, 0x08, 0x02, 0x03, 0x48, 0x2e, };
    modbus_uart_response_t * response = NULL;
    modbus1_init(2);
    uint32_t fake_time = 0;
    fake_time += byte_timing_microseconds(38400) * 1; // 1 byte
    for(uint16_t i=0;i<sizeof(writing_register_address_0x0708_at_slave_2);i++) {
        //Ensure we are not giving a response
        TEST_ASSERT_EQUAL_MESSAGE(NULL, modbus1_get_request(), "modbus request is ready too early");
        modbus1_next_byte(fake_time, writing_register_address_0x0708_at_slave_2[i]);
        fake_time += byte_timing_microseconds(38400) * 1; // 1 byte
    }
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, modbus1_get_request(), "modbus request is ready too early");
    TEST_ASSERT_EQUAL_MESSAGE(CLIENT_STATE_PROCESSING_REQUEST, modbus1_testing->state, "modbus1_testing->state");
    TEST_ASSERT_EQUAL_MESSAGE(0x06, modbus1_testing->payload.request.function, "modbus1_testing->payload.request.function");
    TEST_ASSERT_EQUAL_MESSAGE(0x0708, modbus1_testing->payload.request.address, "modbus1_testing->payload.request.address");
    TEST_ASSERT_EQUAL_MESSAGE(0x0203, modbus1_testing->payload.request.data[0], "modbus1_testing->payload.request.data[0]");

    //Here we say it was successful, and continue on
    response = modbus1_get_response();
    TEST_ASSERT_NOT_EQUAL_MESSAGE(NULL, response, "modbus response is not ready");
    TEST_ASSERT_EQUAL_MESSAGE(8, response->size, "response->size");
}