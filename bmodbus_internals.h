//
// This file is only included in bmodbus.c and unit tests.
//

#ifndef BMODBUS_BMODBUS_INTERNALS_H
#define BMODBUS_BMODBUS_INTERNALS_H
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
    //Payload is outside of this struct so it can be configured differently for each instance
    //uint16_t data[BMB_MAXIMUM_MESSAGE_SIZE];
    modbus_request_t request;
}modbus_client_t;

//This is used to calculate the interbyte delay (if used). It should be 3.5 times the time it takes to send a byte or 1.75ms when > 19200bps
//This calculation should be performed at compile time
#define INTERFRAME_DELAY_MICROSECONDS(BAUDRATE) ((19200 < BAUDRATE) ? 1750 : (35000000 / BAUDRATE))

#ifdef UNIT_TESTING
#ifdef BMB1_CLIENT
extern modbus_client_t * modbus1_testing;
#endif
#endif

#endif //BMODBUS_BMODBUS_INTERNALS_H
