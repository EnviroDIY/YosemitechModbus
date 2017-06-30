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

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    bool begin(yosemitechModel model, byte modbusSlaveID, Stream *stream, int enablePin = -1);

    // This gets the modbus slave ID.  Not supported by many sensors.
    byte getSlaveID(void);

    // This sets a new modbus slave ID
    bool setSlaveID(byte newSlaveID);

    // This gets the instrument serial number as a String
    String getSerialNumber(void);

    // This gets the hardware and software version of the sensor
    // The float variables for the hardware and software versions must be
    // initialized prior to calling this function.
    bool getVersion(float hardwareVersion, float softwareVersion);

    // This tells the optical sensors to begin taking measurements
    bool startMeasurement(void);

    // This tells the optical sensors to stop taking measurements
    bool stopMeasurement(void);

    // This gets values back from the sensor
    // The float variables for value1 and value2 and the byte for the error
    // code must be initialized prior to calling this function.
    bool getValues(float value1, float value2, byte errorCode);

    // This gets the calibration constants for the sensor
    // The float variables for K and B must be
    // initialized prior to calling this function.
    bool getCalibration(float K, float B);

    // This sets the calibration constants for the sensor
    bool setCalibration(float K, float B);

    // This sets the cap coefficients constants for a sensor
    // This only applies to dissolved oxygen sensors
    bool setCapCoefficients(float K0, float K1, float K2, float K3,
                            float K4, float K5, float K6, float K7);

    // This immediately activates the cleaning brush for sensors with one.
    // NOTE:  The brush also activates as soon as power is applied.
    // NOTE:  One cleaning sweep with the brush takes about 10 seconds.
    bool activateBrush(void);

    // This sets the brush interval - that is, how frequently the brush will
    // run if power is continuously applied to the sensor.
    bool setBrushInterval(int intervalMinutes);

    // This returns the brushing interval - that is, how frequently the brush
    // will run if power is continuously applied to the sensor.
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

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}

    // A debugging function for prettily printing raw modbus frames
    // This is purely for debugging
    void printFrameHex(byte modbusFrame[], int frameLength);

    // Calculates a Modbus RTC cyclical redudancy code (CRC)
    // and adds it to the last two bytes of a frame
    void insertCRC(byte modbusFrame[], int frameLength);

    // This sends a command to the sensor bus and listens for a response
    int sendCommand(byte command[], int commandLength);

    int _model;  // The sensor model
    byte _slaveID;  // The sensor slave id
    Stream *_stream;  // The stream instance (serial port) for communication with the RS485
    int _enablePin;  // The pin controlling the driver/receiver enable on the RS485-to-TLL chip

    Stream *_debugStream;  // The stream instance (serial port) for debugging

    int respSize;
    byte responseBuffer[18];  // This needs to be bigger than the largest response

    const uint32_t modbusTimeout = 500;  // The time to wait for response after a command (in ms)
    const int modbusFrameTimeout = 4;  // the time to wait between characters within a frame (in ms)

};



#endif
