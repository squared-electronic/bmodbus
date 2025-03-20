//
// Created by billm on 3/18/2025.
//
#include <stdio.h>
#include <stdint.h>
#include "bmodbus.h"

typedef struct{
    uint32_t placeholder;
}modbus_client_t;

static void bmodbus_init(modbus_client_t *bmodbus){
    printf("bmodbus_init\n");
}
static void bmodbus_deinit(modbus_client_t *bmodbus){
    printf("bmodbus_deinit\n");
}
static void bmodbus_client_next_byte(modbus_client_t *bmodbus, uint32_t microsecont, uint8_t byte){
    printf("bmodbus_client_next_byte\n");
}
static void bmodbus_client_loop(modbus_client_t *bmodbus, uint32_t microsecond){
    printf("bmodbus_client_loop\n");
}

#ifdef BMB1_CLIENT
void modbus1_init(void){
    bmodbus_init(&modbus1);
}
void modbus1_next_byte(uint32_t microseconds, uint8_t byte){
    bmodbus_client_next_byte(&modbus1, microseconds, byte);
}
void modbus1_single_loop(uint32_t microseconds){
    bmodbus_client_loop(&modbus1, microseconds);
}
#endif

