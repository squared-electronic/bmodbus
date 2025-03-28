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

void test_bmodbus_client_simple(void) {
    uint8_t writing_registers_address_0x0708_at_slave_2[] = {0x02, 0x10, 0x07, 0x08, 0x00, 0x02, 0x04, 0x02, 0x03, 0x04, 0x05, 0xe8, 0x06, };
    modbus1_init(2);
    uint32_t fake_time = 0;
    fake_time += byte_timing_microseconds(38400) * 1; // 1 byte
    for(uint16_t i=0;i<sizeof(writing_registers_address_0x0708_at_slave_2);i++) {
        modbus1_next_byte(fake_time, writing_registers_address_0x0708_at_slave_2[i]);
        fake_time += byte_timing_microseconds(38400) * 1; // 1 byte
    }
    TEST_ASSERT_EQUAL_MESSAGE(modbus1_testing->state, CLIENT_STATE_PROCESSING_REQUEST, "modbus1_testing->state");
    //TEST_ASSERT_EQUAL(modbus1_testing->state, CLIENT_STATE_IDLE);
}