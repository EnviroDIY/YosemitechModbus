/*
 *YosemitechModbus.h
*/

#ifndef YosemitechModbus_h
#define YosemitechModbus_h

#include <Arduino.h>
#include <SensorModbusMaster.h>

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
    Y550,  // UV254 Sensor http://www.yosemitech.com/en/product-21.html
    Y4000, //  Multiparameter Sonde http://www.yosemitech.com/en/product-20.html
    UNKNOWN   // Use if the sensor model is unknown. Doing this is generally a
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
    // There is no need to add the & when actually using the function.
    bool getVersion(float &hardwareVersion, float &softwareVersion);

    // This tells the optical sensors to begin taking measurements
    bool startMeasurement(void);

    // This tells the optical sensors to stop taking measurements
    bool stopMeasurement(void);

    // This gets values back from the sensor
    // The float variables for value1 and value2 and the byte for the error
    // code must be initialized prior to calling this function.
    // This function is overloaded so you have the option of getting:
    // 1 value - This will be only the parameter value
    // 1 value and an error code - Parameter value and the error code
    // 2 values - This will be the parameter and the temperature,
    //            with the parameter first and the temperature second
    // 2 values and an error code - As two values, but with error code
    // 3 values - The parameter, the temperature, and a third value for the
    //            sensors that return can return something else
    //            -- Y532 (pH) can return electrical potential
    //            -- Y504 (DO) allows calculation of DO in mg/L, which can be returned
    // 3 values and an error code - As three values, but with error code
    // NOTE:  The one, two, and three value variants will simply return false for a sonde
    bool getValues(float &parmValue);
    bool getValues(float &parmValue, byte &errorCode);
    bool getValues(float &parmValue, float &tempValue);
    bool getValues(float &parmValue, float &tempValue, byte &errorCode);
    bool getValues(float &parmValue, float &tempValue, float &thirdValue);
    bool getValues(float &parmValue, float &tempValue, float &thirdValue, byte &errorCode);
    // 8 values - For Y4000 Sonde, in this order: "DO; Turb; Cond; pH; Temp; ORP; Chl; BGA"
    // 8 values and an error code - As 8 values, but with error code
    // NOTE:  The 8 value versions will return false for anything but a sonde
    bool getValues(float &firstValue, float &secondValue, float &thirdValue, float &forthValue, float &fifthValue, float &sixthValue, float &seventhValue, float &eighthValue); // For Y4000 Sonde
    bool getValues(float &firstValue, float &secondValue, float &thirdValue, float &forthValue, float &fifthValue, float &sixthValue, float &seventhValue, float &eighthValue,  byte &errorCode);
    // This gets the main "parameter" value as a float
    // This is overloaded, so you have the option of getting the error code
    // in another pre-initialized variable, if you want it and the sensor
    // supports it.
    float getValue(void);
    float getValue(byte &errorCode);

    // This returns the temperatures value from a sensor as a float
    float getTemperatureValue(void);

    // This returns raw electrical potential value from the sensor as a float
    // This only applies to pH
    float getPotentialValue(void);

    // This returns DO in mg/L (instead of % saturation) as a float
    // This only applies to DO and is calculated in the getValues() equation using
    // the measured temperature and a salinity of 0 and pressure of 760 mmHg (sea level)
    float getDOmgLValue(void);

    // This gets the calibration constants for the sensor
    // The float variables must be initialized prior to calling this function.
    // MOST sensors have two calibration coefficients:  K = slope, B = intercept
    // The calibration is applied to all values returned by the sensor as:
    //    value_returned = (value_raw * K) + B
    bool getCalibration(float &K, float &B);
    // The pH sensor uses SIX calibration coefficients
    // Factory calibration values for pH are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86, K5=-6.56, K6=-1.04
    bool getCalibration(float &K1, float &K2, float &K3, float &K4, float &K5, float &K6);

    // This sets the calibration constants for a sensor
    // The suggested calibration protocol for sensors with a 2-coefficient calibration is:
    //    1.  Use this command to set calibration coefficients as K = 1 and B = 0
    //    2.  Put the probe in a solution of known value.
    //    3.  Send the "startMeasurement" command and allow the probe to stabilize.
    //    4.  Send the "getValue" command to get the returned parameter value.
    //        (Depending on the sensor, you may want to take multiple values and average them.)
    //    5.  Ideally, repeat steps 2-4 in multiple standard solutions
    //    6.  Calculate the slope (K) and offset (B) between the known values for the standard
    //        solutions and the values returned by the sensor.
    //        (x - values from sensor, y = values of standard solutions)
    //    7.  Send the calculated slope (K) and offset (B) to the sensor using
    //        this command.
    // The pH sensor can be calibrated in this fashion, or it can be calibrated
    // using the steps detailed below for the functions pHCalibrationPoint and pHCalibrationStatus.
    bool setCalibration(float K, float B);

    // This sets the FULL calibration constants for a pH sensor, which requires 6 coefficients
    // Factory calibration values for pH are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86, K5=-6.56, K6=-1.04
    // Use the functions pHCalibrationPoint and pHCalibrationStatus to calibrate
    // and verify calibrations of these meters
    bool setCalibration(float K1, float K2, float K3, float K4, float K5, float K6);

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
    // This only applies to dissolved oxygen sensors.
    // The sensor caps should be replaced yearly or as the readings beome unstable.
    // The values of these coefficients are supplied by the manufacturer.
    bool setCapCoefficients(float K0, float K1, float K2, float K3,
                            float K4, float K5, float K6, float K7);

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
    void setDebugStream(Stream *stream){modbus.setDebugStream(stream);}
    void stopDebugging(void){modbus.stopDebugging();}


private:
    int _model;  // The sensor model

    byte _slaveID;
    modbusMaster modbus;
};

#endif
