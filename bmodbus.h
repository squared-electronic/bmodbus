//
// Created by billm on 3/18/2025.
//

#ifndef BMODBUS_H
#define BMODBUS_H
#include "bmb_config.h"

#ifndef BMB_MAXIMUM_MESSAGE_SIZE
#define BMB_MAXIMUM_MESSAGE_SIZE 128
#endif
//FIXME -- unsure what's the exact number here, but it may change based upon supported commands
#define BMB_MAXIMUM_REGISTER_COUNT ((BMB_MAXIMUM_MESSAGE_SIZE - 7) / 2)

#ifndef BMB1_BAUDRATE
#define BMB1_BAUDRATE 38400
#endif

typedef struct {
    uint8_t function;
    uint8_t size_write;
    int8_t result;
    uint8_t size_read;
    uint16_t data[BMB_MAXIMUM_MESSAGE_SIZE];
}modbus_request_t;

extern void modbus1_init(uint8_t client_address);
extern void modbus1_next_byte(uint32_t microseconds, uint8_t byte);
extern void modbus1_single_loop(uint32_t microseconds);
extern modbus_request_t * modbus1_get_request(void);
extern void modbus1_finish_request(void);
extern void modbus1_finish_send(void);


#endif //BMODBUS_BMODBUS_H
