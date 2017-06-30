/*
 *YosemitechModbus.h
*/

#ifndef YosemitechModbus_h
#define YosemitechModbus_h

#include <Arduino.h>

// The various Yosemitech sensors
typedef enum yosemitechModel
{
    Y502 = 0,  // Online Optical Dissolved Oxygen Sensor http://www.yosemitech.com/en/product-10.html
    Y504,  // Online Optical Dissolved Oxygen Sensor http://www.yosemitech.com/en/product-10.html
    Y510,  // Optical Turbidity Sensor http://www.yosemitech.com/en/product-2.html
    Y511,  // Auto Cleaning Optical Turbidity Sensor http://www.yosemitech.com/en/product-16.html
    Y513,  // Blue Green Algae sensor with Wiper http://www.yosemitech.com/en/product-15.html
    Y514,  // Chlorophyll Sensor with Wiper http://www.yosemitech.com/en/product-14.html
    Y516,  // Oil in water?
    Y520,  // 4-Electrode Conductivity Sensor http://www.yosemitech.com/en/product-3.html
    Y532,  // pH?
    Y533,  // ORP?
    Y550  // UV254 Sensor http://www.yosemitech.com/en/product-21.html
} yosemitechModel;

class yosemitech
{

public:
    bool begin(yosemitechModel model, byte modbusSlaveID, Stream *stream, int enablePin = -1);
    byte getSlaveID(void);
    bool setSlaveID(byte newSlaveID);
    String getSerialNumber(void);
    bool startMeasurement(void);
    bool stopMeasurement(void);
    bool getValues(float value1, float value2, byte ErrorCode);
    bool getVersion(float hardwareVersion, float softwareVersion);
    bool getCalibration(float K, float B);
    bool setCalibration(float K, float B);
    bool setCapCoefficients(float K0, float K1, float K2, float K3,
                            float K4, float K5, float K6, float K7);
    bool activateBrush(void);
    bool setBrushInterval(int intervalMinutes);
    int getBrushInterval(void);



private:

    // Define a small-endian frame as a union - that is a special class type that
    // can hold only one of its non-static data members at a time, in this case,
    // either 4-bytes OR a single float.
    // With avr-gcc (Arduino's compiler), integer and floating point variables are
    // all physically stored in memory in little-endian byte order, so this union
    // is all that is needed to get the correct float value from the small-endian
    // hex frames returned by Yosemitech's Modbus Sensors
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

    // A debugging function for prettily printing raw modbus frames
    void printFrameHex(byte modbusFrame[], int frameLength, Stream *stream = &Serial);

    // Calculates a Modbus RTC cyclical redudancy code (CRC)
    // and adds it to the last two bytes of a frame
    uint16_t insertCRC(byte modbusFrame[], int frameLength);

    int _model;
    int _enablePin;
    Stream *_stream;
    byte _slaveID;
    byte commandBuffer[20];  // This needs to be bigger than the largest response
    byte responseBuffer[20];  // This needs to be bigger than the largest response

};



#endif
