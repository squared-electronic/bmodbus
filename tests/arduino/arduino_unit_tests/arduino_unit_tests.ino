#define FAKE_MAIN
#define UNITY_OMIT_OUTPUT_CHAR_HEADER_DECLARATION
//#define UNITY_OUTPUT_CHAR(x) Serial.write((uint8_t)x)
//#include "../../client/test_modbus_client.c"
#include "test_bmodbus_client.h"

extern "C" void uut_serial_write(const int c){
    Serial.write((uint8_t)c);
    Serial.flush();
}

void setup(){
    Serial.begin(115200);
    Serial.println("Booted");
}

void loop(){
    delay(100);
    Serial.println("Before tests");
    unit_test_main();
    Serial.println("All tests completed");
}