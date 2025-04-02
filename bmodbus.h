//
// Created by billm on 3/18/2025.
//

/**
 * @file bmodbus.h
 * @author Bill McCartney
 * @date 18 Mar 2025
 * @brief File containing definition of api.
 *
 */

#ifndef BMODBUS_H
#define BMODBUS_H
#ifdef __cplusplus
extern "C" {
#endif

#include "bmb_config.h"

#ifndef BMB_MAXIMUM_MESSAGE_SIZE
#define BMB_MAXIMUM_MESSAGE_SIZE 256
#endif
//FIXME -- unsure what's the exact number here, but it may change based upon supported commands
#define BMB_MAXIMUM_REGISTER_COUNT ((BMB_MAXIMUM_MESSAGE_SIZE - 7) / 2)

#ifndef BMB1_BAUDRATE
#define BMB1_BAUDRATE 38400
#endif

typedef struct {
    uint16_t data[BMB_MAXIMUM_MESSAGE_SIZE/2];
    uint16_t size; //It can be a number of registers OR a number of bits
    uint8_t function;
    uint16_t address;
    int8_t result;
}modbus_request_t;

typedef struct {
    uint8_t data[BMB_MAXIMUM_MESSAGE_SIZE];
    uint8_t size; //It is the number of bytes in the response
}modbus_uart_response_t;

/**
 * @brief Initialize the modbus client
 *
 * This initializes the modbus client. It must be called before any other modbus functions.
 * @param client_address - the address of the client 1->254
 * @return none
 * @note This function must be called before any other modbus functions
 */
extern void modbus1_init(uint8_t client_address);
/**
 * @brief send the next byte to the modbus client
 *
 * @param microseconds - the time the last byte was sent in microseconds
 * @param byte - the byte to send
 *
 * @return none
 *
 * @ note: This function should be called for each byte received from the modbus master. It can be called from an interrupt, or the bytes can be sent via a task.
 */
extern void modbus1_next_byte(uint32_t microseconds, uint8_t byte);
extern void modbus1_single_loop(uint32_t microseconds);
/**
 * @ brief: Get the next modbus request if there's one pending
 *
 * @ return: a pointer to the request, or NULL if there's no request
 *
 * @ note: This function is typically called from the main loop to return a request to the application. The application should call modbus_finish_request() when it's done with the request.
 */
extern modbus_request_t * modbus1_get_request(void);
extern modbus_uart_response_t * modbus1_get_response(void);
extern void modbus1_response_send_complete(void);

#ifdef __cplusplus
}
#endif

#endif //BMODBUS_BMODBUS_H
