/*
 *YosemitechModbus.h
*/

#ifndef YosemitechModbus_h
#define YosemitechModbus_h

#include <Arduino.h>

class yosemitech
{
public:

    // Define a small-endian frame as a union - that is a special class type that
    // can hold only one of its non-static data members at a time, in this case,
    // either 4-bytes OR a single float.
    // With avr-gcc (Arduino's compiler), integer and floating point variables are
    // all physically stored in memory in little-endian byte order, so this union
    // is all that is needed to get the correct float value from the small-endian
    // hex frames returned by YosemiTech's Modbus Sensors
    union SeFrame {
      float Float;
      byte Byte[4];
    };

    // This functions return the float from a 4-byte small-endian array beginning
    // at a specific index of another array.
    float floatFromFrame( byte indata[], int stindex);

    // This flips the device/receive enable to DRIVER so the arduino can send text
    void driverEnable(void);

    // This flips the device/receive enable to RECIEVER so the sensor can send text
    void recieverEnable(void);

    // This empties the serial buffer
    void emptyResponseBuffer(Stream *stream);

    // Calculates a Modbus RTC cyclical redudancy code (CRC)
    uint16_t ModRTU_CRC(byte modbusFrame[], int frameLength);

private:
    int DEREPin;

};



#endif
