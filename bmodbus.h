//
// Created by billm on 3/18/2025.
//

#ifndef BMODBUS_H
#define BMODBUS_H


//Here we use defines to see how many instances we need to allocate
#ifdef BMB1_CLIENT
void modbus1_init(void);
void modbus1_next_byte(uint32_t microseconds, uint8_t byte);
void modbus1_single_loop(uint32_t microseconds);
#endif


#endif //BMODBUS_BMODBUS_H
