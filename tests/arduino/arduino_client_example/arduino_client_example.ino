#include "bmodbus.h"

modbus_client_t modbus1;
void setup(){
    bmodbus_client_init(&modbus1, INTERFRAME_DELAY_MICROSECONDS(115200), 2);
    Serial.begin(115200);
}

void loop(){
    if(Serial.available()) {
        uint8_t byte = Serial.read();
        uint32_t last_byte_time = micros(); // Get the time when the last byte was received, so timeouts can be calculated
        modbus_request_t *request = NULL;
        bmodbus_client_next_byte(&modbus1, last_byte_time, byte);
        request = bmodbus_client_get_request(&modbus1);
        if (request) { //Incoming modbus request
            if(dispatch(request->function, request->address, request->data, request->size) == 0){ //Only reply if we accepted the request
                while(Serial.available()){ //Clear the serial buffer if any bytes came in while processing (or were pending)
                    Serial.read();
                }
                modbus_uart_data_t * r = bmodbus_client_get_response(&modbus1); //Generate the payload from the data
                Serial.write(r->data, r->size); //Send the response back to the serial port
            }else{
                request->result = -1; //Indicate an error in the request
                modbus_uart_data_t * r = bmodbus_client_get_response(&modbus1); //Generate an empty payload to reset fsm
            }
        }
    }
}

uint16_t register_values[256] = {0}; // Example register values

static int dispatch(uint8_t function, uint16_t address, uint16_t *data, uint8_t size) {
    // Handle the request based on the function code
    switch (function) {
        case 0x03: // Read Holding Registers
        case 0x04: // Read Input Registers
            // Handle read input registers request
            memset(data, 0, size * 2); // Clear the data buffer
            if(address == 0x1234) {
                data[0] = 0xABCD; // Example value for address 0x1234
            } else if(address == 0x5678) {
                data[0] = 0xEF01; // Example value for address 0x5678
            } else if(address < 256) {
                memcpy(data, &register_values[address], size * 2); // Copy register values
            }
            break;
        case 0x06: // Write Single Register
        case 0x10: // Write Multiple Registers
            // Handle write single/multiple registers request
            if (address < 256) {
                for (int i = 0; i < size; i++) {
                    register_values[address + i] = data[i]; // Store the value in the register
                }
            } else {
                return 0; // Invalid address
            }
            break;
        default:
            return 0; // Unsupported function code
    }
    return 0; // Success
}

