//
// Created by billm on 3/24/2025.
//
#include "unity/unity.h"
#include <stdint.h>
extern void test_bmodbus_client_simple(void);
int main(void) {
    UNITY_BEGIN();

    // Run test cases
    RUN_TEST(test_bmodbus_client_simple);

    return UNITY_END();
}