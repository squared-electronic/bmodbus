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

void test_bmodbus_client_simple(void) {
    modbus1_init();
    TEST_ASSERT_EQUAL(1, 0);
}