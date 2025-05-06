# Bill's Modbus

This is a straightforward modbus RTU implementation in C designed for embedded systems.

It's very lightweight in terms of RAM, Flash and processing. Stop worrying about modbus implementations and focus on your application.

## Testing Status
![Unit Tests Little Endian](https://github.com/ThingDone/bmodbus/actions/workflows/cmake-big-endian-tests.yml/badge.svg)
![Unit Tests Big Endian](https://github.com/ThingDone/bmodbus/actions/workflows/cmake-single-platform.yml/badge.svg)

## Modbus Functionality Supported
This implements both a modbus master and a modbus client (slave) implementation. You can have multiple instances of each type.
We support the following functions:
* Read Coils (0x01)
* Read Discrete Inputs (0x02)
* Read Holding Registers (0x03)
* Read Input Registers (0x04)
* Write Single Coil (0x05)
* Write Single Register (0x06)
* Write Multiple Coils (0x0F)
* Write Multiple Registers (0x10)

Future stuff:
* Documentation
* More Examples
* Live Testing on Physical boards
* Read/Write Multiple Registers (0x17)
* Mask Write Register (0x16)
* Read FIFO Queue (0x18)
* Exception support
* Ascii support
* Intermessage 3.5 char timeout (or 1.75ms when > 19200bps)
* interbyte timeout of 1.5 char times (so if it's >1.5 x char time between bytes discard the message)
* Custom opcode support (it's not hard to add your own, but there isn't a standard way to do it yet)

# Usage
There are three main ways to use this library, and they all share some common elements.
1. Runs in interrupts (requires callbacks and serial writing routine). -- Not tested
2. Runs only in the main loop. Currently tested.
3. In both interrupts and the main loop. -- Not tested

//Describe limitations of each here
| Feature | Only in interrupts | Only in main loop | In both |
|--------|--------------------|-------------------|---------|
| FIXME  | Yes                | No                | Yes     |


All three of these methods assume you have a UART peripheral that is set up and working. 
It also assumes you have a way of getting time in microseconds (at least 32 bits of it). 

## Only in Main Loop
This is the simplest implementation and is easy to do in an arduino or any other environment.
```c
#include "bmodbus.h"

bmodbus_client_t mb;
void setup(){
    Serial.begin(38400);
    //Setup a modbus instance for client/slave address 2
    bmodbus_client_init(&mb, INTERFRAME_DELAY_MICROSECONDS(38400), 2);
}

void loop(){
    if(Serial.available(){
        uint8_t next = Serial.read();
        bmodbus_client_next_byte(&mb, micros(), Serial.read());
        //If there's a message, handle it
        bmodbus_client_request_t * request = bmodbus_client_get_request(&mb);
        if(request != NULL){
            //HANDLE the requests by updating the *request
            ...
            //End of handlers
            
            //Now grab the response in a format the serial port understands
            modbus_uart_data_t * response = bmodbus_client_get_response(&mb);
            Serial.write(response->data, response->length);
            
            //Notify the library that the response has been sent (and we are ready to receive another)
            bmodbus_client_send_complete(&mb); 
        }
    }
}
```

## Only in Interrupts
When we use it via the interrupts we must have the receive interrupt (RX) directly setup properly on the UART peripheral.
This is not trivial in arduinos, since they Arduino libraries don't expose the RX interrupt directly.

This will call the callback methods directly from the interrupt -- allowing your application to be as responsive as possible.
```c
#include "bmodbus.h"


TK


//Call from interrupt
void HARDWARE_UART_RX_ISR(void){
    //Arguments are the current time in microseconds and the next byte received
    bmb1_uart_interrupt_handler(HAL_GetTick()*SOME_CONSTANT, next_byte);
}

int8_t modbus_registers_write(uint16_t address, uint16_t value){
    //Write the value to the address
    switch(address){
        case 0x0000:
            //Do something
            break;
        case 0x0001:
            //Do something else
            break;
        default:
            return -1; //Return -1 if the address is invalid
    }
    return 0; //Return 0 if successful, negative value if not
}

int32_t modbus_registers_read(uint16_t address){
    //Read the value from the address
    switch(address){
        case 0x0000:
            return 0x1234; //Return some value
        case 0x0001:
            return 0x5678; //Return some other value
        default:
            break;
    }
    return -1; //Return -1 if the address is invalid
}

void main(){
    bmb1_init(); //Initializes the modbus library
    while(1){
        //Never have to call any modbus stuff here!
        //Modbus callbacks occur inside of interrupt handlers
    }
}
```


# Story
Bill 3/18/2025 - I've had to implement modbus from scratch for more than twenty projects over the years. Essentially none of them can be open-sourced, so I built the last modbus library I will ever write.

The idea is my customers (and anyone else) can use this library and not have to worry about modbus.
They can focus on their application and not the modbus implementation.

I've made unit tests to cover the functionality.

# Test plan
We currently run our units tests on big endian and little endian systems. In the future we'll run them on processors (or simulators).