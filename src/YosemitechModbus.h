/**
 * @file YosemitechModbus.h
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY YosemitechModbus library for Arduino.
 * @license This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the YosemitechModbus class declarations.
 */

#ifndef YosemitechModbus_h
#define YosemitechModbus_h

#include <Arduino.h>
#include <SensorModbusMaster.h>

/**
 * @brief The various Yosemitech sensors.
 */
typedef enum yosemitechModel {
    Y502 = 0,  ///< [Optical Dissolved Oxygen (discontinued)
               ///< Sensor](http://en.yosemitech.com/aspcms/product/2020-5-8/72.html)
    Y504,      ///< [Optical Dissolved Oxygen (ODO)
               ///< Sensor](https://e.yosemitech.com/DO/Y504-A.html)
    // Y505,  ///< [ Optical Dissolved Oxygen (ODO) for
    //        ///< Aquaculture](https://e.yosemitech.com/DO/Y505-A.html)
    Y510,  ///< [Turbidity
           ///< Sensor](https://e.yosemitech.com/TUR/Y510-B.html)
    Y511,  ///< [Turbidity Sensor with
           ///<  wiper](https://e.yosemitech.com/TUR/Y511-A.html)
    Y513,  ///< [Blue Green Algae (BGA) sensor with
           ///<  Wiper](https://e.yosemitech.com/CHL/Y513-A.html)
    Y514,  ///< [Chlorophyll Sensor with
           ///<  Wiper](https://e.yosemitech.com/CHL/Y514-A.html)
    Y516,  ///< [Oil in water (Crude
           ///< Oil)](http://en.yosemitech.com/aspcms/product/2020-5-8/69.html)
    // Y517,    ///< [Oil in water (Refined
    // Oil)](http://en.yosemitech.com/aspcms/product/2020-5-8/69.html)
    Y520,  ///<  [4-Electrode Conductivity (discontinued)
           ///<  Sensor](http://en.yosemitech.com/aspcms/product/2020-4-23/58.html)
    Y521,  ///< [4-Electrode Conductivity Sensor (metal
           ///< housing)](https://e.yosemitech.com/CT/Y521-A.html)
    Y532,  ///<  [pH Sensor](https://e.yosemitech.com/pH/Y532-A.html)
    Y533,  ///<  [ORP Sensor](https://e.yosemitech.com/pH/Y533-A.html)
    Y550,  ///<  [UV254/COD Sensor (
           ///<  discontinued)](http://en.yosemitech.com/aspcms/product/2020-5-8/94.html)
    Y551,    ///<  [UV254/COD
             ///<  Sensor](https://e.yosemitech.com/COD-16/Y551-B.html)
    Y560,    ///<  [Ammonium ISE Sensor](https://e.yosemitech.com/NH4-N-19/Y560-A.html)
    Y700,    ///<  [Depth Sensor](https://e.yosemitech.com/WLT/68.html)
    Y4000,   ///<  [Multiparameter
             ///<  Sonde](https://e.yosemitech.com/MULTI/Y4000.html)
    UNKNOWN  ///<  Use if the sensor model is unknown. Doing this is generally a bad
             ///<  idea, but it can be helpful for doing things like getting the serial
             ///<  number of an unknown model.
} yosemitechModel;

/**
 * @brief The class for communication with Yosemitech sensors via modbus.
 */
class yosemitech {

 public:

    /**
     * @brief This function sets up the communication.
     *
     * It should be run during the arduino "setup" function.
     * The "stream" device must be initialized prior to running this.
     *
     * @param model The model of the Yosemitech sensor, from #yosemitechModel
     * @param modbusSlaveID The byte identifier of the modbus slave device.
     * @param stream A pointer to the Arduino stream object to communicate with.
     * @param enablePin A pin on the Arduino processor to use to send an enable signal
     * to an RS485 to TTL adapter. Use a negative number if this does not apply.
     * Optional with a default value of -1.
     * @return *bool* True if the starting communication was successful, false if not.
     */
    bool begin(yosemitechModel model, byte modbusSlaveID, Stream* stream,
               int enablePin = -1);
    /**
     * @brief This function sets up the communication.
     *
     * It should be run during the arduino "setup" function.
     * The "stream" device must be initialized prior to running this.
     *
     * @param model The model of the Yosemitech sensor, from #yosemitechModel
     * @param modbusSlaveID The byte identifier of the modbus slave device.
     * @param stream A reference to the Arduino stream object to communicate with.
     * @param enablePin A pin on the Arduino processor to use to send an enable signal
     * to an RS485 to TTL adapter. Use a negative number if this does not apply.
     * Optional with a default value of -1.
     * @return *bool* True if the starting communication was successful, false if not.
     */
    bool begin(yosemitechModel model, byte modbusSlaveID, Stream& stream,
               int enablePin = -1);

    /**
     * @anchor metadata_fxns
     * @name Functions to get and set sensor addresses and metadata
     */
    /**@{*/

    /**
     * @brief Returns a pretty string with the model information
     *
     * @note This is only based on the model input from the "begin" fxn.
     * The sensor itself does not return its model information.
     *
     * @return *String* The Yosemitech sensor model
     */
    String getModel(void);

    /**
     * @brief Returns a pretty string with the parameter measured.
     *
     * @note This is only based on the model input from the "begin" fxn.
     * The sensor itself does not return this information.
     *
     * @return *String* The primary parameter being measured on this Yosemitech sensor
     * model.
     */
    String getParameter(void);

    /**
     * @brief Returns a pretty string with the measurement units.
     *
     * @note This is only based on the model input from the "begin" fxn.
     * The sensor itself does not return this information.
     *
     * @return *String* The units of primary parameter being measured on this Yosemitech
     * sensor model.
     */
    String getUnits(void);

    /**
     * @brief Gets the modbus slave ID.
     *
     * Not supported by many sensors.
     *
     * @return *byte* The slave ID of the Yosemitech sensor
     */
    byte getSlaveID(void);

    /**
     * @brief Sets a new modbus slave ID
     *
     * @param newSlaveID The new slave ID for the Yosemitech sensor
     * @return *bool* True if the slave ID was successfully set, false if not.
     */
    bool setSlaveID(byte newSlaveID);

    /**
     * @brief Gets the instrument serial number as a String
     *
     * @return *String* The serial number of the Yosemitech sensor
     */
    String getSerialNumber(void);

    /**
     * @brief Gets the hardware and software version of the sensor
     *
     * The float variables for the hardware and software versions must be initialized
     * prior to calling this function.
     *
     * The reference (&) is needed when declaring this function so that the function is
     * able to modify the actual input floats rather than create and destroy copies of
     * them.
     *
     * There is no need to add the & when actually using the function.
     *
     * @param hardwareVersion A reference to a float object to be modified with the
     * hardware version.
     * @param softwareVersion A reference to a float object to be modified with the
     * software version.
     * @return *bool* True if the hardware and software versions were successfully
     * updated, false if not.
     */
    bool getVersion(float& hardwareVersion, float& softwareVersion);
    /**@}*/

    /**
     * @anchor measurement_fxns
     * @name Functions to start and stop measurements
     */
    /**@{*/

    /**
     * @brief Tells the optical sensors to begin taking measurements
     *
     * @return *bool* True if the measurements were successfully started, false if not.
     */
    bool startMeasurement(void);

    /**
     * @brief Tells the optical sensors to stop taking measurements
     *
     * @return *bool* True if the measurements were successfully started, false if not.
     */
    bool stopMeasurement(void);
    /**@}*/

    /**
     * @anchor value_fetching
     * @name Functions to get one or more values from a sensor.
     *
     * The float variables for value1 and value2 and the byte for the error
     * code must be initialized prior to calling this function.
     *
     * This function is overloaded so you have the option of getting:
     * 1 value - This will be only the parameter value
     * 1 value and an error code - Parameter value and the error code
     *
     * 2 values - This will be the parameter and the temperature,
     *            with the parameter first and the temperature second
     * 2 values and an error code - As two values, but with error code
     *
     * 3 values - The parameter, the temperature, and a third value for the
     *            sensors that return can return something else
     *            -- Y532 (pH) can return electrical potential
     *            -- Y551 (COD) can return turbidity
     *            -- Y560 (Ammonium) returns NH4_N (mg/L) and pH as primary parameters,
     *            but can return more.
     *            -- Y504 (DO) allows calculation of DO in mg/L, which can be returned
     * 3 values and an error code - As three values, but with error code
     *
     * @note The one, two, and three value variants will simply return false for a
     * multiparameter sonde
     *
     * 8 values - For sondes Y4000 and Y560.
     *          - Y4000 Sonde returns in this order: "DO; Turb; Cond; pH; Temp; ORP;
     *          Chl; BGA"
     *          - Y560 Ammonium returns these groupings of parameters:
     *                 - pH, pH_potential (mV)
     *                 - NH4+_potential (mV), K_potential (mV)
     *                 - NH4_N, NH4+, K+ (all in mg/L)
     *                 - Temperature (C)
     * 8 values and an error code - As 8 values, but with error code
     *
     * @note The 8 value versions will return false for anything but a sonde
     */
    /**@{*/

    /**
     * @brief Gets values back from the sensor
     *
     * @see value_fetching
     *
     * @param parmValue A float to replace with the first parameter value from the
     * sensor.
     * @return  True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& parmValue);
    /**
     * @brief Gets values back from the sensor
     *
     * @see value_fetching
     *
     * @param parmValue A float to replace with the first parameter value from the
     * sensor.
     * @param errorCode A byte to replace with the error code from the measurement.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& parmValue, byte& errorCode);
    /**
     * @brief Gets values back from the sensor
     *
     * @see value_fetching
     *
     * @param parmValue A float to replace with the first parameter value from the
     * sensor.
     * @param tempValue A float to replace with the temperature parameter value from the
     * sensor.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& parmValue, float& tempValue);
    /**
     * @brief Gets values back from the sensor
     *
     * @see value_fetching
     *
     * @param parmValue A float to replace with the first parameter value from the
     * sensor.
     * @param tempValue A float to replace with the temperature parameter value from the
     * sensor.
     * @param errorCode A byte to replace with the error code from the measurement.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& parmValue, float& tempValue, byte& errorCode);
    /**
     * @brief Gets values back from the sensor
     *
     * @see value_fetching
     *
     * @param parmValue A float to replace with the first parameter value from the
     * sensor.
     * @param tempValue A float to replace with the temperature parameter value from the
     * sensor.
     * @param thirdValue A float to replace with the third parameter value from the
     * sensor, if applicable.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& parmValue, float& tempValue, float& thirdValue);
    /**
     * @brief Gets values back from the sensor
     *
     * @see value_fetching
     *
     * @param parmValue A float to replace with the first parameter value from the
     * sensor.
     * @param tempValue A float to replace with the temperature parameter value from the
     * sensor.
     * @param thirdValue A float to replace with the third parameter value from the
     * sensor, if applicable.
     * @param errorCode A byte to replace with the error code from the measurement.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& parmValue, float& tempValue, float& thirdValue,
                   byte& errorCode);

    /**
     * @brief Gets values back from a multi-parameter sonde
     *
     * @note This will return false for anything but a sonde
     *
     * @param firstValue A float to replace with the first parameter value from the
     * sensor.
     * @param secondValue A float to replace with the second parameter value from the
     * sensor.
     * @param thirdValue A float to replace with the third parameter value from the
     * sensor.
     * @param forthValue A float to replace with the fourth parameter value from the
     * sensor.
     * @param fifthValue A float to replace with the fifth parameter value from the
     * sensor.
     * @param sixthValue A float to replace with the sixth parameter value from the
     * sensor.
     * @param seventhValue A float to replace with the seventh parameter value from the
     * sensor.
     * @param eighthValue A float to replace with the eighth parameter value from the
     * sensor.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& firstValue, float& secondValue, float& thirdValue,
                   float& forthValue, float& fifthValue, float& sixthValue,
                   float& seventhValue, float& eighthValue);
    /**
     * @brief Gets values back from a multi-parameter sonde
     *
     * @note This will return false for anything but a sonde
     *
     * @param firstValue A float to replace with the first parameter value from the
     * sensor.
     * @param secondValue A float to replace with the second parameter value from the
     * sensor.
     * @param thirdValue A float to replace with the third parameter value from the
     * sensor.
     * @param forthValue A float to replace with the fourth parameter value from the
     * sensor.
     * @param fifthValue A float to replace with the fifth parameter value from the
     * sensor.
     * @param sixthValue A float to replace with the sixth parameter value from the
     * sensor.
     * @param seventhValue A float to replace with the seventh parameter value from the
     * sensor.
     * @param eighthValue A float to replace with the eighth parameter value from the
     * sensor.
     * @param errorCode A byte to replace with the error code from the measurement.
     * @return *bool* True if the measurements were successfully obtained, false if not.
     */
    bool getValues(float& firstValue, float& secondValue, float& thirdValue,
                   float& forthValue, float& fifthValue, float& sixthValue,
                   float& seventhValue, float& eighthValue, byte& errorCode);
    /**@}*/

    /**
     * @anchor single_values
     * @name Functions to get single values from a sensor
     */
    /**@{*/

    /**
     * @brief Gets the main "parameter" value as a float
     *
     * This is overloaded, so you have the option of getting the error code in another
     * pre-initialized variable, if you want it and the sensor supports it.
     *
     * @return *float* The main "parameter" value as a float
     */
    float getValue(void);
    /**
     * @brief Gets the main "parameter" value as a float
     *
     * @param errorCode A byte to replace with the error code from the measurement.
     * @return *float* The main "parameter" value as a float
     */
    float getValue(byte& errorCode);

    /**
     * @brief Returns the temperatures value from a sensor as a float
     *
     * @return *float* The temperature value as a float
     */
    float getTemperatureValue(void);

    /**
     * @brief Returns raw electrical potential value from the sensor as a float
     *
     * This only applies to pH
     *
     * @return *float* The raw electrical potential value as a float
     */
    float getPotentialValue(void);

    /**
     * @brief Returns DO in mg/L (instead of % saturation) as a float
     *
     * This only applies to DO and is calculated in the getValues() equation using the
     * measured temperature and a salinity of 0 and pressure of 760 mmHg (sea level)
     *
     * @return *float* The dissolved oxygen value as a float
     */
    float getDOmgLValue(void);
    /**@}*/

    /**
     * @anchor calibrations
     * @name Functions get and set sensor calibrations
     */
    /**@{*/

    /**
     * @brief Gets the calibration constants for the sensor
     *
     * The float variables must be initialized prior to calling this function.
     * MOST sensors have two calibration coefficients:  K = slope, B = intercept
     * The calibration is applied to all values returned by the sensor as:
     *    value_returned = (value_raw * K) + B
     *
     * @note This should **NOT** be used with the pH sensor.
     *
     * @param K A float to replace with the first calibration constant - K = slope.
     * @param B A float to replace with the second calibration constant - B =
     * intercept.
     * @return *bool* True if floats were successfully replaced with the calibration
     * information, false if not.
     */
    bool getCalibration(float& K, float& B);
    /**
     * @brief Gets all six calibration constants for a pH sensor
     *
     * The pH sensor uses SIX calibration coefficients
     * Factory calibration values for pH are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86,
     * K5=-6.56, K6=-1.04
     *
     * @note This only applies to the pH sensor!
     *
     * @param K1 A float to replace with the first calibration constant.
     * @param K2 A float to replace with the second calibration constant.
     * @param K3 A float to replace with the third calibration constant.
     * @param K4 A float to replace with the fourth calibration constant.
     * @param K5 A float to replace with the fifth calibration constant.
     * @param K6 A float to replace with the sixth calibration constant.
     * @return *bool* True if floats were successfully replaced with the calibration
     * information, false if not.
     */
    bool getCalibration(float& K1, float& K2, float& K3, float& K4, float& K5,
                        float& K6);

    /**
     * @brief Sets the calibration constants for a sensor
     *
     * The suggested calibration protocol for sensors with a 2-coefficient calibration
     * is:
     *    1.  Use this command to set calibration coefficients as K = 1 and B = 0
     *    2.  Put the probe in a solution of known value.
     *    3.  Send the "startMeasurement" command and allow the probe to stabilize.
     *    4.  Send the "getValue" command to get the returned parameter value.
     *        (Depending on the sensor, you may want to take multiple values and average
     *        them.)
     *    5.  Ideally, repeat steps 2-4 in multiple standard solutions
     *    6.  Calculate the slope (K) and offset (B) between the known values for the
     *    standard
     *        solutions and the values returned by the sensor.
     *        (x - values from sensor, y = values of standard solutions)
     *    7.  Send the calculated slope (K) and offset (B) to the sensor using
     *        this command.
     *
     * The pH sensor can be calibrated in this fashion, or it can be calibrated
     * using the steps detailed below for the functions pHCalibrationPoint and
     * pHCalibrationStatus.
     *
     * @param K The calibration slope
     * @param B The calibration intercept
     * @return *bool* True if the calibration was successfully set; false if not.
     */
    bool setCalibration(float K, float B);

    /**
     * @brief Sets the FULL calibration constants for a pH sensor, which requires 6
     * coefficients.
     *
     * Factory calibration values for pH are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86,
     * K5=-6.56, K6=-1.04.
     *
     * Use the functions yosemitech::pHCalibrationPoint(float pH) and
     * yosemitech::pHCalibrationStatus() to calibrate and verify calibrations of these
     * meters
     *
     * @param K1 The first calibration constant.
     * @param K2 The second calibration constant.
     * @param K3 The third calibration constant.
     * @param K4 The fourth calibration constant.
     * @param K5 The fifth calibration constant.
     * @param K6 The sixth calibration constant.
     * @return *bool* True if the calibration was successfully set; false if not.
     */
    bool setCalibration(float K1, float K2, float K3, float K4, float K5, float K6);

    /**
     * @brief Sets the 3 calibration points for a pH sensor
     *
     * Calibration steps for pH (3 point calibration only):
     *   1. Put sensor in solution and allow to stabilize for 1 minute
     *   2. Input value of calibration standard (ie, run command pHCalibrationPoint(pH))
     *   3. Repeat for points 2 and 3 (pH of 4.00, 6.86, and 9.18 recommended)
     *   4. Read calibration status
     *
     * @param pH The pH of the current calibration solution.
     * @return *bool* True if the calibration point was accepted; false if not.
     */
    bool pHCalibrationPoint(float pH);

    /**
     * @brief Verifies the success of a calibration for a pH sensor
     *
     * Return values:
     *   0x00 - Success
     *   0x01 - Non-matching calibration standards
     *   0x02 - Less than 3 points used in calibration
     *   0x04 - Calibration coefficients out of range
     *
     * @return *byte* A byte with the calibration status.
     */
    byte pHCalibrationStatus(void);

    /**
     * @brief Sets the cap coefficients constants for a sensor
     *
     * This only applies to dissolved oxygen sensors.
     * The sensor caps should be replaced yearly or as the readings become unstable.
     * The values of these coefficients are supplied by the manufacturer.
     *
     * @param K0 The zero-th DO cap coefficient.
     * @param K1 The first DO cap coefficient.
     * @param K2 The second DO cap coefficient.
     * @param K3 The third DO cap coefficient.
     * @param K4 The fourth DO cap coefficient.
     * @param K5 The fifth DO cap coefficient.
     * @param K6 The sixth DO cap coefficient.
     * @param K7 The seventh DO cap coefficient.
     * @return *bool* True if the coefficients were accepted; false if not.
     */
    bool setCapCoefficients(float K0, float K1, float K2, float K3, float K4, float K5,
                            float K6, float K7);
    /**@}*/

    /**
     * @anchor brushing
     * @name Functions for sensor brushes
     */
    /**@{*/

    /**
     * @brief Immediately activates the cleaning brush for sensors with one.
     *
     * @note The brush also activates as soon as power is applied.
     * @note One cleaning sweep with the brush takes about 10 seconds.
     * @note Brushing commands will only work on turbidity sensors with
     * hardware Rev1.0 and software Rev1.7 or later
     *
     * @return *bool* True if the brush was activated; false if not.
     */
    bool activateBrush(void);

    /**
     * @brief Sets the brush interval - that is, how frequently the brush will run if
     * power is continuously applied to the sensor.
     *
     * @note Brushing commands will only work on turbidity sensors with hardware Rev1.0
     * and software Rev1.7 or later
     *
     * @param intervalMinutes The brushing interval in minutes
     * @return *bool* True if the brushing interval was successfully set; false if not
     */
    bool setBrushInterval(uint16_t intervalMinutes);

    /**
     * @brief Returns the brushing interval - that is, how frequently the brush will run
     * if power is continuously applied to the sensor.
     *
     * @note Brushing commands will only work on turbidity sensors with hardware Rev1.0
     * and software Rev1.7 or later
     *
     * @return *uint16_t* The brushing interval in minutes
     */
    uint16_t getBrushInterval(void);
    /**@}*/

    /**
     * @anchor debugging
     * @name Debugging functions
     */
    /**@{*/

    /**
     * @brief Set a stream for debugging information to go to.
     *
     * @param stream An Arduino stream object
     */
    void setDebugStream(Stream* stream) {
        modbus.setDebugStream(stream);
    }
    /**
     * @copydoc yosemitech::setDebugStream(Stream* stream)
     */
    void setDebugStream(Stream& stream) {
        modbus.setDebugStream(stream);
    }
    /**
     * @brief Un-set the stream for debugging information to go to; stop debugging.
     */
    void stopDebugging(void) {
        modbus.stopDebugging();
    }
    /**@}*/


 private:
    int  _model;    ///< the sensor model
    byte _slaveID;  ///< the sensor slave id

    modbusMaster modbus;  ///< an internal reference to the modbus communication object.
};

#endif
