/**
 * @file bmodbus.h
 * @author Bill McCartney
 * @date 18 Mar 2025
 * @brief File containing definition of api.
 *
 */

/* MIT Style License
 * Copyright (c) 2025 Bill McCartney

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BMODBUS_H
#define BMODBUS_H
#ifdef __cplusplus
extern "C" {
#endif

#define BMODBUS_VERSION_MAJOR   (1)
#define BMODBUS_VERSION_MINOR   (0)

//This can be used to modify the maximum message size to something other than 256 bytes -- it reduces the RAM required
#ifndef BMB_MAXIMUM_MESSAGE_SIZE
#define BMB_MAXIMUM_MESSAGE_SIZE 256
#endif
//FIXME -- unsure what's the exact number here, but it may change based upon supported commands
#define BMB_MAXIMUM_REGISTER_COUNT ((BMB_MAXIMUM_MESSAGE_SIZE - 7) / 2)

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
}modbus_uart_data_t;

typedef enum{
    CLIENT_NO_INIT=0, CLIENT_STATE_IDLE, CLIENT_STATE_WAITING_FOR_NEXT_MESSAGE, CLIENT_STATE_FUNCTION_CODE, CLIENT_STATE_HEADER, CLIENT_STATE_HEADER_CHECK, CLIENT_STATE_DATA, CLIENT_STATE_FOOTER, CLIENT_STATE_FOOTER2, CLIENT_STATE_PROCESSING_REQUEST, CLIENT_STATE_SENDING_RESPONSE
}modbus_client_state_t;

typedef union{
    uint16_t word[2];
    uint8_t byte[4];
}header_t;
typedef union{
    uint16_t half;
    uint8_t byte[2];
}uint16_bytes;

typedef struct{
    modbus_client_state_t state;
    uint32_t last_microseconds;
    uint32_t interframe_delay;
    uint8_t client_address; //historically called slave address
    //These are active function variables used in headers
    header_t header;
    uint16_bytes crc;
    uint8_t function;
#ifdef BMB_CLIENT_ASCII
    uint8_t ascii; //Used for modbus ascii
#endif //BMB_CLIENT_ASCII
    uint16_t byte_count; //Used for keeping track of message length
#ifdef BMB_CLIENT_READ_WRITE_FUNCTION //These are only needed if we implement the read-write function
    uint16_t address2;
    uint8_t size2;
#endif //BMB_CLIENT_READ_WRITE_FUNCTION
    uint8_t index;
    uint8_t byte_size;
    //Payload is outside of this struct so it can be configured differently for each instance
    union{
        modbus_request_t request;
        modbus_uart_data_t response;
    }payload;

}modbus_client_t;


/**
 * @brief Initialize the modbus client
 *
 * This initializes the modbus client. It must be called before any other modbus functions.
 * @param bmodbus - the modbus client instance to initialize
 * @param client_address - the address of the client 1->254
 * @param interframe_delay - the time in microseconds between bytes. This can be generated from the baudrate INTERFRAME_DELAY_MICROSECONDS(38400)
 * @return none
 * @note This function must be called before any other modbus functions
 * @example
 *    modbus_client_t modbus1;
 *    bmodbus_client_init(&modbus1, INTERFRAME_DELAY_MICROSECONDS(38400), client_address);
 */
extern void bmodbus_client_init(modbus_client_t *bmodbus, uint32_t interframe_delay, uint8_t client_address);
/**
 * @brief send the next byte to the modbus client
 *
 * @param bmodbus - the modbus client instance
 * @param microseconds - the time the last byte was sent in microseconds
 * @param byte - the byte to send
 *
 * @return none
 *
 * @note: This function should be called for each byte received from the modbus master. It can be called from an interrupt, or the bytes can be sent via a task.
 */
extern void bmodbus_client_next_byte(modbus_client_t *bmodbus, uint32_t microseconds, uint8_t byte);
extern void bmodbus_client_loop(modbus_client_t *bmodbus, uint32_t microsecond);
/**
 * @brief Get the next modbus request if there's one pending
 *
 * @param bmodbus - the modbus client instance
 * @return a pointer to the request, or NULL if there's no request
 *
 * @note This function is typically called from the main loop to return a request to the application. The application should call modbus_finish_request() when it's done with the request.
 */
extern modbus_request_t * bmodbus_client_get_request(modbus_client_t *bmodbus);
/**
 * @brief Get the bytes to send to the serial port after the request processing is complete.
 * @param bmodbus - the modbus client instance
 * @return a pointer to the response, or NULL if there's no response
 */
extern modbus_uart_data_t * bmodbus_client_get_response(modbus_client_t * bmodbus);
/**
 * @brief Update the state machine
 *
 * @param bmodbus - the modbus client instance
 * @return none
 *
 * @note This function should be called when the response is sent. It will reset the state machine and prepare for the next request.
 */
extern void bmodbus_client_send_complete(modbus_client_t * bmodbus);

/**
 * @brief Deinitialize the modbus client
 *
 * @param bmodbus
 * @return none
 *
 * @note This function should be called when the modbus client is no longer needed.
 */
extern void bmodbus_client_deinit(modbus_client_t *bmodbus);


typedef enum{
    MASTER_NO_INIT=0, MASTER_STATE_IDLE, MASTER_STATE_SENDING_REQUEST, MASTER_STATE_WAITING_FOR_RESPONSE, MASTER_STATE_PROCESSING_RESPONSE, MASTER_STATE_RESPONSE_READY
}modbus_master_state_t;

typedef struct{
    uint8_t data[BMB_MAXIMUM_MESSAGE_SIZE];
    uint8_t size; //It is the number of bytes in the response
    uint8_t expected_response_size;
}modbus_uart_request_t;

typedef struct{
    modbus_master_state_t state;
    uint32_t interframe_delay;
    uint32_t last_microseconds;
    uint16_bytes crc;
    uint8_t client_address; //historically called slave address
    uint16_t register_address;
    uint8_t function;
    uint8_t byte_count;
    union{
        modbus_request_t response;
        modbus_uart_request_t request;
    }payload;
}modbus_master_t;

extern void bmodbus_master_init(modbus_master_t *bmodbus, uint32_t interframe_delay);
extern void bmodbus_master_send_complete(modbus_master_t * bmodbus, uint32_t microseconds);
extern void bmodbus_master_next_byte(modbus_master_t *bmodbus, uint32_t microseconds, uint8_t byte);
extern void bmodbus_master_received(modbus_master_t *bmodbus, uint32_t microseconds, uint8_t * bytes, uint8_t length, uint32_t microseconds_per_byte);
extern modbus_request_t * bmodbus_master_get_response(modbus_master_t *bmodbus);
extern modbus_uart_request_t * bmodbus_master_read_coils(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count);
extern modbus_uart_request_t * bmodbus_master_read_discrete_inputs(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count);
extern modbus_uart_request_t * bmodbus_master_read_holding_registers(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count);
extern modbus_uart_request_t * bmodbus_master_read_input_registers(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count);
extern modbus_uart_request_t * bmodbus_master_write_single_coil(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t value);
extern modbus_uart_request_t * bmodbus_master_write_single_register(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t value);
extern modbus_uart_request_t * bmodbus_master_write_multiple_coils(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t count, uint8_t *data);
extern modbus_uart_request_t * bmodbus_master_write_multiple_registers(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t count, uint16_t *data);

//Utility for calculating the minimum interfame delay -- which is the time from receiving the last byte of the request to sending the first byte of the response
//This is used to calculate the interbyte delay (if used). It should be 3.5 times the time it takes to send a byte or 1.75ms when > 19200bps
#define INTERFRAME_DELAY_MICROSECONDS(BAUDRATE) ((19200 < BAUDRATE) ? 1750 : (35000000 / BAUDRATE))

//Utility for calculating the byte timing -- which is used when buffers are transfered.
#define BYTE_TIMING_IN_MICROSECONDS(BAUDRATE) (1000000 / ((BAUDRATE)/10))

#ifdef __cplusplus
}
#endif

#endif //BMODBUS_BMODBUS_H
