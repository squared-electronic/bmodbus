/* MIT Style License

Copyright (c) 2025 Bill McCartney

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

#include <stdio.h>
#include <stdint.h>
#include "bmodbus.h"
/* Headers of requests:
 * 1 read coils = 2 byte starting address, 2 byte quantity of coils
 * 2 read discrete inputs = 2 byte starting address, 2 byte quantity of inputs
 * 3 read holding registers = 2 byte starting address, 2 byte quantity of registers
 * 4 read input registers = 2 byte starting address, 2 byte quantity of registers
 * 5 write single coil = 2 byte address, 2 byte value
 * 6 write single register = 2 byte address, 2 byte value
 * 15 write multiple coils = 2 byte starting address, 2 byte quantity of coils, 1 byte byte count, N bytes of data  (header is only 4 bytes)
 * 16 write multiple registers = 2 byte starting address, 2 byte quantity of registers, 1 byte byte count, N bytes of data (header is only 4 bytes)
 */

#ifndef MODBUS_HTONS
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define MODBUS_HTONS(x) ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define MODBUS_FIRST_BYTE(X) ((uint8_t)((X) >> 8))
#define MODBUS_SECOND_BYTE(X) ((uint8_t)(0xff & X))
#else
#define MODBUS_HTONS(x) (x)
#define MODBUS_FIRST_BYTE(X) ((uint8_t)(0xff & X))
#define MODBUS_SECOND_BYTE(X) ((uint8_t)((X) >> 8))
#endif
#endif

#ifndef MODBUS_MEMMOVE
static void modbus_memmove(void * dst, void * src, uint16_t count) {
    if(dst < src) { //If dest is before source, we can copy forward
        for(uint16_t i = 0; i<count; i++) {
            ((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
        }
        return;
    }else {
        for (int16_t i = (count) - 1; i >= 0; i--) {
            ((uint8_t *) dst)[i] = ((uint8_t *) src)[i];
        }
    }
}
#define MODBUS_MEMMOVE modbus_memmove
#endif
#define MODBUS_UNUSED(x) (void)(x)

void bmodbus_client_init(modbus_client_t *bmodbus, uint32_t interframe_delay, uint8_t client_address){
    bmodbus->state = CLIENT_STATE_IDLE;
    bmodbus->interframe_delay = interframe_delay;
    bmodbus->client_address = client_address;
    bmodbus->crc.half = 0xFFFF;
    bmodbus->byte_count = 0;
}

void bmodbus_client_deinit(modbus_client_t *bmodbus){
    printf("bmodbus_client_deinit\n");
    bmodbus->state = CLIENT_NO_INIT;
}

static inline uint8_t process_request(modbus_client_t *bmodbus){
    //Returns true iff the request was processed successfully via events
    MODBUS_UNUSED(bmodbus);
#ifdef MODBUS1_CALLBACK
    if(bmodbus == &modbus1){
        return MODBUS1_CALLBACK(bmodbus);
    }
#endif //MODBUS1_CALLBACK
    return 0;
}

static uint16_t crc_update(uint16_t crc, uint8_t byte) {
    uint16_t poly = 0xA001;
    crc = (uint16_t)(crc ^ byte);
    for (uint8_t i = 0; i < 8; i++) {
        if (crc & 1) {
            crc = (crc >> 1) ^ poly;
        } else {
            crc = (crc >> 1);
        }
    }
    return crc;
}

void bmodbus_client_next_byte(modbus_client_t *bmodbus, uint32_t microseconds, uint8_t byte){
    //If the time delta is greater than the interframe delay, we should reset the state machine, and then process from scratch
    if((microseconds - bmodbus->last_microseconds) > bmodbus->interframe_delay){
        bmodbus->state = CLIENT_STATE_IDLE;
        bmodbus->byte_count = 0;
        bmodbus->crc.half = 0xFFFF;
    }
    bmodbus->last_microseconds = microseconds;
    switch(bmodbus->state){
        case CLIENT_STATE_IDLE:
            if(byte == bmodbus->client_address){
                bmodbus->state = CLIENT_STATE_FUNCTION_CODE;
            }else{
                //FIXME we should add optional tracking of errors for debug purposes
                //FIXME future work, we can add broadcast here
                bmodbus->state = CLIENT_STATE_WAITING_FOR_NEXT_MESSAGE;
            }
            break;
        case CLIENT_STATE_FUNCTION_CODE:
            bmodbus->function = byte;
            bmodbus->state = CLIENT_STATE_HEADER;
            break;
        case CLIENT_STATE_HEADER:
            bmodbus->header.byte[bmodbus->byte_count-2] = byte;
            if(bmodbus->byte_count == 5){
                //Endianness conversion
                bmodbus->header.word[0] = MODBUS_HTONS(bmodbus->header.word[0]);
                bmodbus->header.word[1] = MODBUS_HTONS(bmodbus->header.word[1]);
                if(bmodbus->function == 16) { //These are the only functions that have a byte count
                    bmodbus->state = CLIENT_STATE_HEADER_CHECK;
                    bmodbus->byte_size = 2 * bmodbus->header.word[1]; //Bytes for number of registers
                    //FIXME -- ensure byte_size cannot be more than 250
                }else if(bmodbus->function == 15){
                    bmodbus->state = CLIENT_STATE_HEADER_CHECK;
                    bmodbus->byte_size = (bmodbus->header.word[1] + 7) / 8;//Bytes for target number of bits
                    //FIXME -- ensure byte_size cannot be more than 250
                }else{
                    bmodbus->state = CLIENT_STATE_FOOTER;
                }
            }
            break;
        case CLIENT_STATE_HEADER_CHECK:
            if(byte == bmodbus->byte_size) { //It contains a byte count and we compare it with 2x the number of 16bit registers
                bmodbus->index = 0;
                bmodbus->state = CLIENT_STATE_DATA;
            }else{
                //FIXME we should add optional tracking of errors for debug purposes
                bmodbus->state = CLIENT_STATE_WAITING_FOR_NEXT_MESSAGE;
            }
            break;
        case CLIENT_STATE_DATA:
            ((uint8_t*)bmodbus->payload.request.data)[bmodbus->index] = byte;
            if(bmodbus->index & 1){ //Endianness conversion every completed word
                bmodbus->payload.request.data[bmodbus->index/2] = MODBUS_HTONS(bmodbus->payload.request.data[bmodbus->index/2]);
            }
            bmodbus->index++;
            if(bmodbus->index == bmodbus->byte_size){
                //Here we can process the request
                bmodbus->state = CLIENT_STATE_FOOTER;
                bmodbus->index = 0;
            }
            break;
        case CLIENT_STATE_FOOTER:
            bmodbus->crc.half = MODBUS_HTONS(bmodbus->crc.half);
            //Here we verify the CRC
            if(bmodbus->crc.byte[1] == byte){
                bmodbus->state = CLIENT_STATE_FOOTER2;
            }else{
                //FIXME bad CRC
                bmodbus->state = CLIENT_STATE_WAITING_FOR_NEXT_MESSAGE;
            }
            break;
        case CLIENT_STATE_FOOTER2:
            if(bmodbus->crc.byte[0] == byte){
                //FIXME --  either process the request OR just wait for a polling entry (pending request)
                bmodbus->payload.request.function = bmodbus->function;
                bmodbus->payload.request.address = bmodbus->header.word[0];
                //Here we prep the request struct for higher layers
                switch (bmodbus->function) {
                    case 6:
                        bmodbus->payload.request.size = 1;
                        bmodbus->payload.request.data[0] = bmodbus->header.word[1];
                        break;
                    case 15:
                    case 16:
                    case 3:
                    case 4:
                    case 2:
                    case 1:
                        bmodbus->payload.request.size = bmodbus->header.word[1];
                        break;
                    default:
                        break;
                }
                bmodbus->payload.request.result = 0;
                bmodbus->state = CLIENT_STATE_PROCESSING_REQUEST;
                if(process_request(bmodbus)){
                    //event processing was complete
                }
            }else{
                //FIXME bad CRC
                bmodbus->state = CLIENT_STATE_WAITING_FOR_NEXT_MESSAGE;
            }
            break;
        default: //Ignore bytes in all other states
            break;
    }
    bmodbus->byte_count++;
    if((bmodbus->state == CLIENT_STATE_FUNCTION_CODE) || (bmodbus->state == CLIENT_STATE_HEADER) || (bmodbus->state == CLIENT_STATE_HEADER_CHECK) || (bmodbus->state == CLIENT_STATE_DATA) || (bmodbus->state == CLIENT_STATE_FOOTER)){
        bmodbus->crc.half = crc_update(bmodbus->crc.half, byte);
    }
}

void bmodbus_client_loop(modbus_client_t *bmodbus, uint32_t microsecond){
    //FIXME -- currently not implemented
    MODBUS_UNUSED(bmodbus);
    MODBUS_UNUSED(microsecond);
    printf("bmodbus_client_loop\n");
}

static void bmodbus_encode_client_response(modbus_client_t *bmodbus){
    uint16_t temp1, temp2;
    int i;
    //This takes the request and encodes it into the response (assuming processing is completed)
    switch (bmodbus->function){
        case 6:
        case 16:
            //If failed return no response
            if(bmodbus->payload.request.result){
                bmodbus->payload.response.size = 0;
                break;
            }
            //Store values from the request for the response
            temp1 = bmodbus->payload.request.size;
            temp2 = bmodbus->payload.request.address;
            bmodbus->payload.response.size = 6;
            bmodbus->payload.response.data[2] = (temp2 & 0xFF00) >> 8;
            bmodbus->payload.response.data[3] = temp2 & 0xFF;
            bmodbus->payload.response.data[4] = (temp1 & 0xFF00) >> 8;
            bmodbus->payload.response.data[5] = temp1 & 0xFF;
            break;
        case 3:
        case 4:
            //If failed return no response
            if(bmodbus->payload.request.result){
                bmodbus->payload.response.size = 0;
                break;
            }
            //Store values from the request for the response
            temp1 = bmodbus->payload.request.size;
            //Endian flip the results
            for(i=0;i<temp1;i++){
                bmodbus->payload.request.data[i] = MODBUS_HTONS(bmodbus->payload.request.data[i]);
            }
            //Move the payload data to the response at the offset
            MODBUS_MEMMOVE(bmodbus->payload.response.data+3, bmodbus->payload.request.data, 2*temp1);
            //FIXME above move could overflow buffer on a weird read request
            bmodbus->payload.response.size = 3 + 2*temp1;
            bmodbus->payload.response.data[2] = 2*temp1;
            break;
        case 1:
        case 2:
            //If failed return no response
            if(bmodbus->payload.request.result){
                bmodbus->payload.response.size = 0;
                break;
            }
            //Store values from the request for the response
            temp1 = (bmodbus->payload.request.size + 7) / 8; //Number of bytes from number of bits
            //Move the payload data (bytes) to the response at the offset
            MODBUS_MEMMOVE(bmodbus->payload.response.data+3, bmodbus->payload.request.data, temp1);
            //FIXME above move could overflow buffer on a weird read request
            bmodbus->payload.response.size = 3 + temp1;
            bmodbus->payload.response.data[2] = temp1;
            break;
        case 15:
            //If failed return no response
            if(bmodbus->payload.request.result){
                bmodbus->payload.response.size = 0;
                break;
            }
            //Store values from the request for the response
            temp1 = bmodbus->payload.request.size;
            temp2 = bmodbus->payload.request.address;
            bmodbus->payload.response.size = 6;
            bmodbus->payload.response.data[2] = (temp2 & 0xFF00) >> 8;
            bmodbus->payload.response.data[3] = temp2 & 0xFF;
            bmodbus->payload.response.data[4] = (temp1 & 0xFF00) >> 8;
            bmodbus->payload.response.data[5] = temp1 & 0xFF;
            break;
    }
    if(bmodbus->payload.response.size) {
        uint16_t response_crc = 0xFFFF;
        //All responses start the same...
        bmodbus->payload.response.data[0] = bmodbus->client_address;
        bmodbus->payload.response.data[1] = bmodbus->function;
        //Calculate the CRC
        for (i = 0; i < bmodbus->payload.response.size; i++) {
            response_crc = crc_update(response_crc, bmodbus->payload.response.data[i]);
        }
        bmodbus->payload.response.data[bmodbus->payload.response.size] = response_crc & 0xFF;
        bmodbus->payload.response.data[bmodbus->payload.response.size + 1] = (response_crc & 0xFF00) >> 8;
        bmodbus->payload.response.size += 2;
    }
}

modbus_request_t * bmodbus_client_get_request(modbus_client_t * bmodbus){
    if(bmodbus->state == CLIENT_STATE_PROCESSING_REQUEST){
        return &(bmodbus->payload.request);
    }
    return NULL;
}

modbus_uart_data_t * bmodbus_client_get_response(modbus_client_t * bmodbus){
    if(bmodbus->state == CLIENT_STATE_PROCESSING_REQUEST){
        //Here we process the request data structure into the UART response
        bmodbus_encode_client_response(bmodbus);
        bmodbus->state = CLIENT_STATE_SENDING_RESPONSE;
        return &(bmodbus->payload.response);
    }else if(bmodbus->state == CLIENT_STATE_SENDING_RESPONSE){
        return &(bmodbus->payload.response);
    }
    return NULL;
}

void bmodbus_client_send_complete(modbus_client_t * bmodbus){
    //This is called when the response has been sent
    if(bmodbus->state == CLIENT_STATE_SENDING_RESPONSE){
        bmodbus->state = CLIENT_STATE_IDLE;
        bmodbus->byte_count = 0;
        bmodbus->crc.half = 0xFFFF;
    }
}

void bmodbus_master_init(modbus_master_t *bmodbus, uint32_t interframe_delay){
    bmodbus->state = MASTER_STATE_IDLE;
    bmodbus->interframe_delay = interframe_delay;
    bmodbus->crc.half = 0xFFFF;
    bmodbus->byte_count = 0;
}

void bmodbus_master_next_byte(modbus_master_t *bmodbus, uint32_t microseconds, uint8_t byte){

}

modbus_uart_request_t * modbus_master_send_internal(modbus_master_t *bmodbus, uint8_t client_address, uint8_t function, uint16_t start_address, uint16_t value_or_count, uint16_t * data, uint8_t expected){
    int i;
    uint16_t crc = 0xFFFF;
    bmodbus->client_address = client_address;
    bmodbus->register_address = start_address;
    bmodbus->function = function;
    bmodbus->byte_count = 0;
    bmodbus->payload.request.data[0] = client_address;
    crc = crc_update(crc, client_address);
    bmodbus->payload.request.data[1] = function;
    crc = crc_update(crc, function);
    if(function == 16) { //value contains count in these functions
        bmodbus->payload.request.data[2] = (start_address & 0xFF00) >> 8;
        bmodbus->payload.request.data[3] = start_address & 0xFF;
        bmodbus->payload.request.data[4] = (value_or_count & 0xFF00) >> 8;
        bmodbus->payload.request.data[5] = value_or_count & 0xFF;
        bmodbus->payload.request.data[6] = value_or_count * 2;
        for(i=2; i<6; i++){
            crc = crc_update(crc, bmodbus->payload.request.data[i]);
        }
        for (i = 0; i < value_or_count; i++) {
            uint8_t temp = (data[i] & 0xFF00) >> 8;
            bmodbus->payload.request.data[i * 2 + 7] = temp;
            crc = crc_update(crc, function);
            temp = data[i] & 0xFF;
            bmodbus->payload.request.data[i * 2 + 8] = temp;
        }
        uint16_bytes crc_bytes;
        crc_bytes.half = MODBUS_HTONS(crc);
        bmodbus->payload.request.data[value_or_count * 2 + 7] = crc_bytes.byte[1];
        bmodbus->payload.request.data[value_or_count * 2 + 8] = crc_bytes.byte[0];
        bmodbus->payload.request.size = value_or_count * 2 + 9;
    }else if(function == 15){
        bmodbus->payload.request.data[2] = (start_address & 0xFF00) >> 8;
        bmodbus->payload.request.data[3] = start_address & 0xFF;
        bmodbus->payload.request.data[4] = (value_or_count & 0xFF00) >> 8;
        bmodbus->payload.request.data[5] = value_or_count & 0xFF;
        bmodbus->payload.request.data[6] = (value_or_count + 7) / 8; //Number of bytes from number of bits
        for(i=2; i<7; i++){
            crc = crc_update(crc, bmodbus->payload.request.data[i]);
        }
        for (i = 0; i < (value_or_count + 7) / 8; i++) {
            bmodbus->payload.request.data[i + 7] = ((uint8_t*)data)[i];
            crc = crc_update(crc, function);
        }
    }else{
        uint8_t temp;
        temp = MODBUS_FIRST_BYTE(start_address);
        crc = crc_update(crc, temp);
        bmodbus->payload.request.data[2] = temp;
        temp = MODBUS_SECOND_BYTE(start_address);
        crc = crc_update(crc, temp);
        bmodbus->payload.request.data[3] = temp;
        temp = MODBUS_FIRST_BYTE(value_or_count);
        crc = crc_update(crc, temp);
        bmodbus->payload.request.data[4] = temp;
        temp = MODBUS_SECOND_BYTE(value_or_count);
        crc = crc_update(crc, temp);
        bmodbus->payload.request.data[5] = temp;
        uint16_bytes crc_bytes;
        crc_bytes.half = MODBUS_HTONS(crc);
        bmodbus->payload.request.data[6] = crc_bytes.byte[1];
        bmodbus->payload.request.data[7] = crc_bytes.byte[0];
        bmodbus->payload.request.size = 8;
        bmodbus->payload.request.expected_response_size = expected;
    }
    return &(bmodbus->payload.request);
}

modbus_uart_request_t * bmodbus_master_read_coils(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count){
    return modbus_master_send_internal(bmodbus, client_address, 1, start_address, count, NULL, (count+7)/8 + 5);
}
modbus_uart_request_t * bmodbus_master_read_discrete_inputs(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count){
    return modbus_master_send_internal(bmodbus, client_address, 2, start_address, count, NULL, (count+7)/8 + 5);
}
modbus_uart_request_t * bmodbus_master_read_holding_registers(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count){
    return modbus_master_send_internal(bmodbus, client_address, 3, start_address, count, NULL, count*2 + 5);
}
modbus_uart_request_t * bmodbus_master_read_input_registers(modbus_master_t *bmodbus, uint8_t client_address, uint16_t start_address, uint16_t count){
    return modbus_master_send_internal(bmodbus, client_address, 4, start_address, count, NULL, count*2 + 5);
}
modbus_uart_request_t * bmodbus_master_write_single_coil(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t value){
    return modbus_master_send_internal(bmodbus, client_address, 5, address, value, NULL, 8);
}
modbus_uart_request_t * bmodbus_master_write_single_register(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t value){
    return modbus_master_send_internal(bmodbus, client_address, 6, address, value, NULL, 8);
}
modbus_uart_request_t * bmodbus_master_write_multiple_coils(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t count, uint8_t *data){
    return modbus_master_send_internal(bmodbus, client_address, 15, address, count, (uint16_t*)data, 8);
}
modbus_uart_request_t * bmodbus_master_write_multiple_registers(modbus_master_t *bmodbus, uint8_t client_address, uint16_t address, uint16_t count, uint16_t *data){
    return modbus_master_send_internal(bmodbus, client_address, 16, address, count, data, 8);
}