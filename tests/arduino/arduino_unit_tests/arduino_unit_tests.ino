#define FAKE_MAIN
#define UNITY_OMIT_OUTPUT_CHAR_HEADER_DECLARATION
//#define UNITY_OUTPUT_CHAR(x) Serial.write((uint8_t)x)
//#include "../../client/test_modbus_client.c"
#include "test_bmodbus_client.h"

extern "C" void uut_serial_write(int c){
    Serial.write((uint8_t)c);
}

void setup(){
    Serial.begin(115200);
}

void loop(){
    delay(1000);
    unit_test_main();
    Serial.println("All tests completed");
}