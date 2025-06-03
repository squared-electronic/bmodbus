//
// Configures unity with specifics for the arduino.
//

#ifndef BMODBUS_UNITY_CONFIG_H
#define BMODBUS_UNITY_CONFIG_H
#define UNITY_OUTPUT_CHAR uut_serial_write
void uut_serial_write(const int x);
#endif //BMODBUS_UNITY_CONFIG_H
