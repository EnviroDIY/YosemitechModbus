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
    Y532,  // pH
    Y533,  // ORP?
    Y550,   // UV254 Sensor http://www.yosemitech.com/en/product-21.html
    UNKNOWN   //  Use if the sensor model is unknown. Doing this is generally a
              // bad idea, but it can be helpful for doing things like getting
              // the serial number of an unknown model.
} yosemitechModel;

class yosemitech
{

public:

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    bool begin(yosemitechModel model, byte modbusSlaveID, Stream *stream, int enablePin = -1);
    bool begin(yosemitechModel model, byte modbusSlaveID, Stream &stream, int enablePin = -1);

    // This returns a pretty string with the model information
    // NOTE:  This is only based on the model input from the "begin" fxn.
    // The sensor itself does not return its model information.
    String getModel(void);

    // This returns a pretty string with the parameter measured.
    // NOTE:  This is only based on the model input from the "begin" fxn.
    // The sensor itself does not return this information.
    String getParameter(void);

    // This returns a pretty string with the measurement units.
    // NOTE:  This is only based on the model input from the "begin" fxn.
    // The sensor itself does not return this information.
    String getUnits(void);

    // This gets the modbus slave ID.  Not supported by many sensors.
    byte getSlaveID(void);

    // This sets a new modbus slave ID
    bool setSlaveID(byte newSlaveID);

    // This gets the instrument serial number as a String
    String getSerialNumber(void);

    // This gets the hardware and software version of the sensor
    // The float variables for the hardware and software versions must be
    // initialized prior to calling this function.
    // The reference (&) is needed when declaring this function so that
    // the function is able to modify the actual input floats rather than
    // create and destroy copies of them.
    // There is no need to add the & when actually usig the function.
    bool getVersion(float &hardwareVersion, float &softwareVersion);

    // This tells the optical sensors to begin taking measurements
    bool startMeasurement(void);

    // This tells the optical sensors to stop taking measurements
    bool stopMeasurement(void);

    // This gets values back from the sensor
    // The float variables for value1 and value2 and the byte for the error
    // code must be initialized prior to calling this function.
    // This function is overloaded so you have the option of getting 1 value
    // 2 values, or 2 values and the error code.
    // For all of the sensors I have manuals for, other than pH, value1
    // is the temperature and value2 is the "other" value.
    bool getValues(float &value1);
    bool getValues(float &value1, float &value2);
    bool getValues(float &value1, float &value2, byte &errorCode);

    // This gets raw electrical potential values back from the sensor
    // This only applies to pH
    // The float variable for value1 must be initialized prior to calling this function.
    bool getPotentialValue(float &value1);

    // This gets the temperatures value from a sensor
    // The float variable for value1 must be initialized prior to calling this function.
    bool getTemperatureValue(float &value1);

    // This gets the calibration constants for the sensor
    // The float variables for K and B must be
    // initialized prior to calling this function.
    bool getCalibration(float &K, float &B);

    // This sets the calibration constants for the sensor
    // This is for all sensors EXCEPT pH
    bool setCalibration(float K, float B);

    // This sets the 3 calibration points for a pH sensor
    // Calibration steps for pH (3 point calibration only):
    //   1. Put sensor in solution and allow to stabilize for 1 minute
    //   2. Input value of calibration standard (ie, run command pHCalibrationPoint(pH))
    //   3. Repeat for points 2 and 3 (pH of 4.00, 6.86, and 9.18 recommended)
    //   4. Read calibration status
    bool pHCalibrationPoint(float pH);

    // This verifies the success of a calibration for a pH sensor
    // Return values:
    //   0x00 - Success
    //   0x01 - Non-matching calibration standards
    //   0x02 - Less than 3 points used in calibration
    //   0x04 - Calibration coefficients out of range
    byte pHCalibrationStatus(void);

    // This sets the cap coefficients constants for a sensor
    // This only applies to dissolved oxygen sensors
    bool setCapCoefficients(float K0, float K1, float K2, float K3,
                            float K4, float K5, float K6, float K7);

    // This sets the calibration constants for a pH sensor
    // Factory calibration values are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86, K5=-6.56, K6=-1.04
    bool setpHCalibration(float K1, float K2, float K3,
                          float K4, float K5, float K6);

    // This immediately activates the cleaning brush for sensors with one.
    // NOTE:  The brush also activates as soon as power is applied.
    // NOTE:  One cleaning sweep with the brush takes about 10 seconds.
    // NOTE:  Brushing commands will only work on turbidity sensors with
    // hardware Rev1.0 and software Rev1.7 or later
    bool activateBrush(void);

    // This sets the brush interval - that is, how frequently the brush will
    // run if power is continuously applied to the sensor.
    // NOTE:  Brushing commands will only work on turbidity sensors with
    // hardware Rev1.0 and software Rev1.7 or later
    bool setBrushInterval(uint16_t intervalMinutes);

    // This returns the brushing interval - that is, how frequently the brush
    // will run if power is continuously applied to the sensor.
    // NOTE:  Brushing commands will only work on turbidity sensors with
    // hardware Rev1.0 and software Rev1.7 or later
    uint16_t getBrushInterval(void);

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}


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

    // This is just as above, but for a 2-byte interger
    union SeFrame2 {
      uint16_t Int;
      byte Byte[2];
    };

    // This functions return the float from a 4-byte small-endian array beginning
    // at a specific index of another array.
    float floatFromFrame(byte indata[], int stindex);

    // This functions inserts a float as a 4-byte small endian array into another
    // array beginning at the specified index.
    void floatIntoFrame(byte indata[], int stindex, float value);

    // This flips the device/receive enable to DRIVER so the arduino can send text
    void driverEnable(void);

    // This flips the device/receive enable to RECIEVER so the sensor can send text
    void recieverEnable(void);

    // This empties the serial buffer
    void emptyResponseBuffer(Stream *stream);

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

    // This creates a null stream to use for "debugging" if you don't want to
    // actually print to a real stream.
    struct NullStream : public Stream
    {
        NullStream( void ) { return; }
        int available( void ) { return 0; }
        void flush( void ) { return; }
        int peek( void ) { return -1; }
        int read( void ){ return -1; }
        size_t write( uint8_t u_Data ){ return 0; }
        size_t write(const uint8_t *buffer, size_t size) { return 0; }
    };
    NullStream nullstream;
    Stream *_debugStream = &nullstream;  // The stream instance (serial port) for debugging

    int respSize;
    byte responseBuffer[18];  // This needs to be bigger than the largest response

    // The modbus protocol defines that there can be no more than 1.5 characters
    // of silence between characters in a frame and any space over 3.5 characters
    // defines a new frame.
    // At 9600 baud with 1 start bit, 8 data bits, no parity check, and 2 stop bits
    // (the transmission mode for all Yosemitech sensors) 1 character takes ~1.14ms
    // So the readBytes() command should time out within 4ms
    const uint32_t modbusTimeout = 500;  // The time to wait for response after a command (in ms)
    const int modbusFrameTimeout = 4;  // the time to wait between characters within a frame (in ms)

    static float junk_val;
    static byte junk_byte;

};

#endif
