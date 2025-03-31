//
// Created by billm on 3/24/2025.
//
#include "unity/unity.h"
#include <stdint.h>
extern void test_client_simple_write(void);
extern void test_client_single_write(void);
extern void test_client_single_read(void);
extern void test_client_multiple_read(void);

int main(void) {
    UNITY_BEGIN();

    // Run test cases
    RUN_TEST(test_client_simple_write);
    RUN_TEST(test_client_single_write);
    RUN_TEST(test_client_single_read);
    RUN_TEST(test_client_multiple_read);

    return UNITY_END();
}