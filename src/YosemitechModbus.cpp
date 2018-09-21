/*
 *YosemitechModbus.cpp
*/

#include "YosemitechModbus.h"


//----------------------------------------------------------------------------
//                          PUBLIC SENSOR FUNCTIONS
//----------------------------------------------------------------------------


// This function sets up the communication
// It should be run during the arduino "setup" function.
// The "stream" device must be initialized and begun prior to running this.
bool yosemitech::begin(yosemitechModel model, byte modbusSlaveID, Stream *stream, int enablePin)
{
    // Give values to variables;
    _model = model;
    _slaveID = modbusSlaveID;
    // Start up the modbus instance
    bool success = modbus.begin(modbusSlaveID, stream, enablePin);
    // Get the model type from the serial number if it's not known
    if (_model == UNKNOWN) getSerialNumber();

    return success;
}
bool yosemitech::begin(yosemitechModel model, byte modbusSlaveID, Stream &stream, int enablePin)
{return begin(model, modbusSlaveID, &stream, enablePin);}


// This returns a pretty string with the model information
String yosemitech::getModel(void)
{
    switch (_model)
    {
        case Y502: {return "Y502";}
        case Y504: {return "Y504";}
        case Y510: {return "Y510";}
        case Y511: {return "Y511";}
        case Y513: {return "Y513";}
        case Y514: {return "Y514";}
        case Y516: {return "Y516";}
        case Y520: {return "Y520";}
        case Y532: {return "Y532";}
        case Y533: {return "Y533";}
        case Y550: {return "Y550";}
        case Y4000: {return "Y4000";}
        default:  {return "Unknown";}
    }
}


// This returns a pretty string with the parameter measured.
String yosemitech::getParameter(void)
{
    switch (_model)
    {
        case Y502: {return "Dissolved Oxygen";}
        case Y504: {return "Dissolved Oxygen";}
        case Y510: {return "Turbidity";}
        case Y511: {return "Turbidity";}
        case Y513: {return "Blue Green Algae";}
        case Y514: {return "Chlorophyll";}
        case Y516: {return "Oil in Water";}
        case Y520: {return "Conductivity";}
        case Y532: {return "pH";}
        case Y533: {return "ORP";}
        case Y550: {return "COD";}
        case Y4000: {return "DO,   Turb, Cond,  pH,   Temp, ORP,  Chl,  BGA";}
        default:  {return "Unknown";}
    }
}


// This returns a pretty string with the parameter measured.
String yosemitech::getUnits(void)
{
    switch (_model)
    {
        case Y502: {return "percent";}
        case Y504: {return "percent";}
        case Y510: {return "NTU";}
        case Y511: {return "NTU";}
        case Y513: {return "µg/L";}
        case Y514: {return "µg/L";}
        case Y516: {return "ppb";}
        case Y520: {return "mS/cm";}
        case Y532: {return "pH";}
        case Y533: {return "mV";}
        case Y550: {return "mg/L";}
        case Y4000: {return "mg/L, NTU,  mS/cm, pH,   °C,   mV,   µg/L, µg/L";}
        default:  {return "Unknown";}
    }
}


// This gets the modbus slave ID.  Not supported by many sensors.
// TODO:  Get list of YosemiTech sensors this works for
// The slaveID is in register 0x3000 (12288)
byte yosemitech::getSlaveID(void)
{
    byte command[8] = {0xFF, 0x03, 0x30, 0x00, 0x00, 0x01, 0x9E, 0xD4};
    int respSize = modbus.sendCommand(command, 8);

    if (respSize == 7) return modbus.responseBuffer[3];
    else return 0x01;  // This is the default address
}


// This sets a new modbus slave ID
// The slaveID is in register 0x3000 (12288)
bool yosemitech::setSlaveID(byte newSlaveID)
{
    byte dataToSend[2] = {newSlaveID, 0x00};
    return modbus.setRegisters(0x3000, 1, dataToSend, true);
}


// This gets the instrument serial number as a String
// Serial number begins in holding register 0x0900 (2304) and occupies 7 registers (14 characters)
String yosemitech::getSerialNumber(void)
{
    String SN;
    switch (_model)
    {
        case Y4000:
            SN = modbus.StringFromRegister(0x03, 0x1400, 14); break; // for Y4000 Sonde
        default:
            SN = modbus.StringFromRegister(0x03, 0x0900, 14); break; // for all sensors except Y4000
    }

    // Strip out the initial ')' that seems to come with some responses
    if (SN.startsWith(")"))
    {
        SN = SN.substring(1);
    }

    // Verify model and serial number match
    // Serial number to model information based on personal communication with Yosemitech
    // TODO:  Get serial numbers for the rest of the sensors
    int modelSS = SN.substring(2,4).toInt();

    // If model was unknown, assign it based on serial number
    if (_model == UNKNOWN)
    {
        if (modelSS == 1) _model = Y504;  // 01 means DO sensor
        if (modelSS == 9) _model = Y520;  // 09 means conductivity sensor
        if (modelSS == 10) _model = Y510;  // 10 means turbidity sensor
        if (modelSS == 29) _model = Y511;  // 29 means self-cleaning turbidity sensor
        if (modelSS == 48) _model = Y514;  // 48 means chlorophyll
        if (modelSS == 43) _model = Y532;  // 43 must mean pH
        if (modelSS == 38) _model = Y4000;  // 38 must mean MultiParameter Sonde
    }

    /*
    // Print warnings when model and serial number do not match
    if (modelSS == 1 && (_model != Y502 && _model !=Y504))  // 01 means DO sensor
        _debugStream->print(F("Serial number and model number do not match!"));
    if (modelSS == 9 && (_model != Y520))  // 09 means conductivity sensor
        _debugStream->print(F("Serial number and model number do not match!"));
    if (modelSS == 10 && (_model != Y510))  // 10 means turbidity sensor
        _debugStream->print(F("Serial number and model number do not match!"));
    if (modelSS == 29 && (_model != Y511))  // 29 means self-cleaning turbidity sensor
        _debugStream->print(F("Serial number and model number do not match!"));
    if (modelSS == 48 && (_model != Y514))  // 48 means chlorophyll
        _debugStream->print(F("Serial number and model number do not match!"));
    if (modelSS == 43 && (_model != Y532))  // 43 must mean pH
        _debugStream->print(F("Serial number and model number do not match!"));
    */

    // Return the serial number
    return SN;
}


// This gets the hardware and software version of the sensor
// This data begins in holding register 0x0700 (1792) and continues for 2 registers
bool yosemitech::getVersion(float &hardwareVersion, float &softwareVersion)
{
    // Parse into version numbers
    // These aren't actually little endian responses.  The first byte is the
    // major version and the second byte is the minor version.
    if (modbus.getRegisters(0x03, 0x0700, 2))
    {
        hardwareVersion = modbus.byteFromFrame(3) + (float)modbus.byteFromFrame(4) / 100;
        softwareVersion = modbus.byteFromFrame(5) + (float)modbus.byteFromFrame(6) / 100;
        return true;
    }
    else return false;
}


// This tells the optical sensors to begin taking measurements
// Note: this doesn't appear to be necessary for the Y4000 sonde
bool yosemitech::startMeasurement(void)
{
    switch (_model)
    {
        case Y520:
        {
            byte startMeasurementW[9] = {_slaveID, 0x10, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Write, Reg 7168 ,0 Registers, 0byte,    CRC
            int respSize = modbus.sendCommand(startMeasurementW, 9);
            if (respSize == 8 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
        }
        case Y4000: // Does not require this function. Not described in the manual or sent using the MultiSensor_v1.18 software
        {
            return true;
        }
        default:
        {
            byte startMeasurementR[8] = {_slaveID, 0x03, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Read,  Reg 9472 ,   0 Regs  ,    CRC
            int respSize = modbus.sendCommand(startMeasurementR, 8);
            if (respSize == 5 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
        }
    }
}


// This tells the optical sensors to stop taking measurements
bool yosemitech::stopMeasurement(void)
{
    switch (_model)
    {
        case Y4000: // Does not require this function. Not described in the manual or sent using the MultiSensor_v1.18 software
        {
            return true;
        }
        default:
        {
            // byte stopMeasurement[8] = {_slaveID, 0x03, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00};
                                       // _slaveID, Read,  Reg 11776,   0 Regs  ,    CRC
            byte stopMeasurement[8] = {_slaveID, 0x03, 0x2E, 0x00, 0x00, 0x01, 0x00, 0x00};
                                    // _slaveID, Read,  Reg 11776,   1 Reg   ,    CRC
            int respSize = modbus.sendCommand(stopMeasurement, 8);
            // if (respSize == 5 && modbus.responseBuffer[0] == _slaveID) return true;
            if (respSize == 7 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
        }
    }
}


// This gets values back from the sensor
// All sensors but pH, return two 32-bit float values beginning in holding register
// 0x2600 (9728), where the parameter value is in the first two register and the
// temperature in celsius is in the next two registers.  For some sensors
// (Y520/Conductivity and Y514/Chlorophyll) this followed by an error code.
// The pH sensor returns the pH as a 32-bit float beginning in holding register
// 0x2800 (10240) and the temperature in celsuis as a separate 32-bit float
// beginning in holding register 0x2400 (9216).  The pH sensor can also return
// the raw electrical potential values back from the sensor as a 32-bit float
// beginning in holding register 0x1200 (4608).  As a convienence, I am also
// calculating the DO in mg/L from the DO sensor, which otherwise would only
// return percent saturation.
bool yosemitech::getValues(float &parmValue, float &tempValue, float &thirdValue, byte &errorCode)
{
    // Set values to -9999 and error flagged before asking for the result
    parmValue = -9999;
    tempValue = -9999;
    thirdValue = -9999;
    errorCode = 0xFF;  // Error!

    switch (_model)
    {
        case Y4000:   // Y4000 Multiparameter sonde
        {
            // The sonde returns 8 values at once, we're not going to pick three
            // of them to return.  We'll just send a false response.  If someone
            // wants the sonde results, they should give 8 values to put them in.
            return false;
        }
        case Y550:   // Y550 COD, with turbidity
        {
            if (modbus.getRegisters(0x03, 0x2600, 5))
            {
                tempValue = modbus.float32FromFrame(littleEndian, 3);
                parmValue = modbus.float32FromFrame(littleEndian, 7);
                errorCode = modbus.byteFromFrame(11);
                thirdValue = modbus.float32FromRegister(0x03,  0x1200, littleEndian);
                return true;
            }
            break;
        }
        // Y532 (pH) or Y533 (ORP)
        case Y532:
        case Y533:
        {
            if (modbus.getRegisters(0x03, 0x2800, 2))
            {
                parmValue = modbus.float32FromFrame(littleEndian, 3);
                tempValue = modbus.float32FromRegister(0x03,  0x2400, littleEndian);
                thirdValue = modbus.float32FromRegister(0x03,  0x1200, littleEndian);
                errorCode = 0x00;  // No errors
                return true;
            }
            break;
        }
        // Y504 (DO)
        case Y502:
        case Y504:
        {
            if (modbus.getRegisters(0x03, 0x2600, 4))
            {
                tempValue = modbus.float32FromFrame(littleEndian, 3);
                float DOpercent = modbus.float32FromFrame(littleEndian, 7);
                parmValue = DOpercent * 100;  // Because it returns number not %
                errorCode = 0x00;  // No errors

                // Calculate DO saturation at sea level at a given temp/salinity
                // using equation by Weiss (1970, Deep-Sea Res. 17:721-735)
                //
                // The equation by Weiss reads:
                //
                // ln DO = A1 + A2 100/T + A3 ln T/100 + A4 T/100          (1)
                //
                //    + S [B1 + B2 T/100 + B3 (T/100)2]
                //
                // where:
                //   ln DO is the natural log of the DO solubility in milliliters per liter (ml/L)
                //   T = temperature in degrees K(273.15 + t  degrees C)
                //   S = salinity in g/kg (o/oo)
                     float A1 = -173.4292;
                     float A2 = 249.6339;
                     float A3 = 143.3483;
                     float A4 = - 21.8492;
                     float Bl = - 0.033096;
                     float B2 =   0.014259;
                     float B3 = - 0.001700;

                //  Calculate DO saturation at sea level at a given temp/salinity
                float Tkelvin = 273.15 + tempValue; //  celsius to kelvin
                float salinity = 0.0;  // assume 0 for pure water
                float lnDO = A1 + A2*(100/Tkelvin) + A3*log(Tkelvin/100) + A4*(Tkelvin/100)
                             + salinity*(Bl + B2*(Tkelvin/100) + B3*(Tkelvin/100)*(Tkelvin/100));
                 float DO_saturation_SL_mlL = exp(lnDO);

                //  Multiply by the constant 1.4276 to
                //  convert to milligrams per liter (mg/L).
                 float DO_saturation_SL_mgL = DO_saturation_SL_mlL*1.4276;

                //  Calculate the vapor pressur of water at sea level at a given
                //  temperature from the empirical equation derived from the
                //  Handbook of Chemistry and Physics
                //  (Chemical Rubber Company, Cleveland, Ohio, 1964)
                //
                //  log u = 8.10765 - (1750.286/ (235+t))                   (3)
                //  where:
                //    t is temperature in degrees C
                //    log u is the log base 10 of the vapor pressur of water in mmHg
                float logVaporPressureH2O = 8.10765 - (1750.286/(235+tempValue));
                float VaporPressureH2O = pow(logVaporPressureH2O, 10);

                // Correct the DO saturation for the vapor pressure of water
                // at pressures other than sea level using the equation:
                // DO' = D0! (P-u/760-u)                                    (2)
                //
                // where:
                //   DO' is the saturation DO at barometric pressure P
                //   D0! is saturation DO at barometric pressure 760 mm Hg
                //   u is the vapor pressure of water
                float baroPressure_mmHg = 760;  // assume working at sea level
                float DO_saturation_press_mgL = DO_saturation_SL_mgL*
                      ((baroPressure_mmHg - VaporPressureH2O) / (760 - VaporPressureH2O));

                // Finally, multiply the measured percent saturation by the mg/L
                // concentration of O2 at saturation at the given temperature,
                // pressure, and salinity to get the measured DO concentration in mg/L
                float DOmgL = DO_saturation_press_mgL*DOpercent;
                thirdValue = DOmgL;

                return true;
            }
            break;
        }
        // Everybody else other than Y550 COD; Y532 (pH) or Y533 (ORP); Y502 & Y504 (DO)
        default:
        {
            if (modbus.getRegisters(0x03, 0x2600, 5))
            {
                tempValue = modbus.float32FromFrame(littleEndian, 3);
                parmValue = modbus.float32FromFrame(littleEndian, 7);
                errorCode = modbus.byteFromFrame(11);
                return true;
            }
            break;
        }
    }

    // If something fails, we'll get here
    return false;
}
bool yosemitech::getValues(float &parmValue, float &tempValue, float &thirdValue)
{
    byte errorCode = 0xFF;  // Initialize as if there's an error
    return getValues(parmValue, tempValue, thirdValue, errorCode);
}
bool yosemitech::getValues(float &parmValue, float &tempValue, byte &errorCode)
{
    float thirdValue = -9999;  // Initialize with an error value
    return getValues(parmValue, tempValue, thirdValue, errorCode);
}
bool yosemitech::getValues(float &parmValue, float &tempValue)
{
    byte errorCode = 0xFF;  // Initialize as if there's an error
    return getValues(parmValue, tempValue, errorCode);
}
bool yosemitech::getValues(float &parmValue, byte &errorCode)
{
    float tempValue = -9999;  // Initialize with an error value
    return getValues(parmValue, tempValue, errorCode);
}
bool yosemitech::getValues(float &parmValue)
{
    byte errorCode = 0xFF;  // Initialize as if there's an error
    return getValues(parmValue, errorCode);
}

// Get 8 values for the multiparameter sonde, with or without error flag
bool yosemitech::getValues(float &DOmgL, float &Turbidity, float &Cond,
                           float &pH, float &Temp, float &ORP,
                           float &Chlorophyll, float &BGA, byte &errorCode)
{
    // Set values to -9999 and error flagged before asking for the result
    DOmgL = -9999;      // firstValue
    Turbidity = -9999;  // secondValue
    Cond = -9999;       // thirdValue
    pH = -9999;         // forthValue
    Temp = -9999;       // fifthValue
    ORP = -9999;        // sixthValue
    Chlorophyll = -9999; // seventhValue
    BGA = -9999;        // eighthValue
    errorCode = 0xFF;  // Error!

    switch (_model)
    {
        case Y4000:   // Y4000 Multiparameter sonde
        {
            // Sonde's 8 values begin in register 260
            if (modbus.getRegisters(0x03, 0x2601, 10))
            {
                DOmgL   = modbus.float32FromFrame(littleEndian, 3);   // DOmgL
                Turbidity = modbus.float32FromFrame(littleEndian, 7);   // Turbidity
                Cond  = modbus.float32FromFrame(littleEndian, 11);  // Conductivity
                pH  = modbus.float32FromFrame(littleEndian, 15);  // pH
                Temp   = modbus.float32FromFrame(littleEndian, 19);  // Temperature
                ORP  = modbus.float32FromFrame(littleEndian, 23);  // ORP
                Chlorophyll = modbus.float32FromFrame(littleEndian, 27); // Chlorophyll
                BGA = modbus.float32FromFrame(littleEndian, 31);  // Blue Green Algae (BGA)
                // Error code is separately stored in register 0x0800
                errorCode = modbus.byteFromRegister(0x03, 0x0800, 1);
                return true;
            }
            break;
        }
        // Only the sonde can return 8 values!
        default: return false;
    }
    // If something fails, we'll get here
    return false;
}
bool yosemitech::getValues(float &firstValue, float &secondValue, float &thirdValue,
                           float &forthValue, float &fifthValue, float &sixthValue,
                           float &seventhValue, float &eighthValue)
{
    byte errorCode = 0xFF;  // Initialize as if there's an error
    return getValues(firstValue, secondValue, thirdValue, forthValue,
                     fifthValue, sixthValue, seventhValue, eighthValue, errorCode);
}


// This returns the main "parameter" value as a float
// NOTE:  This will return -9999 for a sonde!
float yosemitech::getValue(void)
{
    float parmValue = -9999;  // Initialize with an error value
    getValues(parmValue);
    return parmValue;
}
float yosemitech::getValue(byte &errorCode)
{
    float parmValue = -9999;  // Initialize with an error value
    getValues(parmValue, errorCode);
    return parmValue;
}


// This returns the temperatures value from a sensor as a float
float yosemitech::getTemperatureValue(void)
{
    switch (_model)
    {
        case Y4000:
        {
            // Initialize with an error value
            float firstValue, secondValue, thirdValue, forthValue,
                  fifthValue, sixthValue, seventhValue, eighthValue = -9999;
            getValues(firstValue, secondValue, thirdValue, forthValue,
                      fifthValue, sixthValue, seventhValue, eighthValue);
            return fifthValue;  // temp is the 5th value returned
        }
        default:
        {
            float parmValue, tempValue = -9999;  // Initialize with an error value
            getValues(parmValue, tempValue);
            return tempValue;  // temp is the 2nd value for everything else
        }
    }
}

// This returns the raw electrical potential from a pH sensor as a float
float yosemitech::getPotentialValue(void)
{
    switch(_model)
    {
        case Y532:
        case Y533:
        {
            float parmValue, tempValue, thirdValue = -9999;  // Initialize with an error value
            getValues(parmValue, tempValue, thirdValue);
            return thirdValue;
        }
        default:
        {
            return -9999;
        }
    }
}


// This returns DO in mg/L (instead of % saturation) as a float
// This only applies to DO and is calculated in the getValues() equation using
// the measured temperature and a salinity of 0 and pressure of 760 mmHg (sea level)
float yosemitech::getDOmgLValue(void)
{
    switch(_model)
    {
        case Y502:
        case Y504:
        {
            float parmValue, tempValue, thirdValue = -9999;  // Initialize with an error value
            getValues(parmValue, tempValue, thirdValue);
            return thirdValue;
        }
        case Y4000:
        {
            float firstValue, secondValue, thirdValue, forthValue,
                  fifthValue, sixthValue, seventhValue, eighthValue = -9999;
            getValues(firstValue, secondValue, thirdValue, forthValue,
                      fifthValue, sixthValue, seventhValue, eighthValue);
            return firstValue;  // DO in mg/L is the 1st value returned
        }
        default: return -9999;
    }
}


// This gets the calibration constants for a sensor
// For MOST sensors, the K value begins in register 0x1100 (4352) and the B value two registers later
// For pH sensors, the calibration constants begin at register 0x2900 (10496)
// For ORP sensors, the calibration constants begin at register 0x3400 (10496)
// NOTE: skipping programing calibration features for the Y4000 Sonde
bool yosemitech::getCalibration(float &K1, float &K2, float &K3,
                                float &K4, float &K5, float &K6)
{
    switch (_model)
    {
        case Y532:   // pH
        {
            if (modbus.getRegisters(0x03, 0x2900, 12))
            {
                K1 = modbus.float32FromFrame(littleEndian, 3);
                K2 = modbus.float32FromFrame(littleEndian, 7);
                K3 = modbus.float32FromFrame(littleEndian, 11);
                K4 = modbus.float32FromFrame(littleEndian, 15);
                K5 = modbus.float32FromFrame(littleEndian, 19);
                K6 = modbus.float32FromFrame(littleEndian, 23);
                return true;
            }
            else return false;
        }
        case Y533:   // ORP
        {
            if (modbus.getRegisters(0x03, 0x3400, 4))
            {
                K1 = modbus.float32FromFrame(littleEndian, 3);
                K2 = modbus.float32FromFrame(littleEndian, 7);
                return true;
            }
            else return false;
        }
        case Y4000:
        {
            return false;
        }
        default:   // other sensors have only 2 values
        {
            K3 = -9999;
            K4 = -9999;
            K5 = -9999;
            K6 = -9999;
            if (modbus.getRegisters(0x03, 0x1100, 4))
            {
                K1 = modbus.float32FromFrame(littleEndian, 3);
                K2 = modbus.float32FromFrame(littleEndian, 7);
                return true;
            }
            else return false;
        }
    }
}
bool yosemitech::getCalibration(float &K, float &B)
{
    float K3, K4, K5, K6;
    return getCalibration(K, B, K3, K4, K5, K6);
}

// This sets the calibration constants for the sensor
// The suggested calibration protocol is:
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
// The K value begins in register 0x1100 (4352) and the B value two registers later
bool yosemitech::setCalibration(float K, float B)
{
    switch (_model)
    {
        case Y533:  // ORP
        {
            byte calibs[8] = {0x00,};
            modbus.float32ToFrame(K, littleEndian, calibs, 0);
            modbus.float32ToFrame(B, littleEndian, calibs, 4);
            return modbus.setRegisters(0x3400, 4, calibs, true);
        }
        case Y4000:
        {
            return false;
        }
        default:
        {
            byte calibs[8] = {0x00,};
            modbus.float32ToFrame(K, littleEndian, calibs, 0);
            modbus.float32ToFrame(B, littleEndian, calibs, 4);
            return modbus.setRegisters(0x1100, 4, calibs, true);
        }
    }
}

// This sets the calibration constants for a pH sensor
// Factory calibration values for pH are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86, K5=-6.56, K6=-1.04
// The calibration constants begin at register 0x2900 (10496)
bool yosemitech::setCalibration(float K1, float K2, float K3,
                                float K4, float K5, float K6)
{
    byte pHCalibs[24] = {0x00,};
    modbus.float32ToFrame(K1, littleEndian, pHCalibs, 0);
    modbus.float32ToFrame(K2, littleEndian, pHCalibs, 4);
    modbus.float32ToFrame(K3, littleEndian, pHCalibs, 8);
    modbus.float32ToFrame(K4, littleEndian, pHCalibs, 12);
    modbus.float32ToFrame(K5, littleEndian, pHCalibs, 16);
    modbus.float32ToFrame(K6, littleEndian, pHCalibs, 20);
    return modbus.setRegisters(0x2900, 12, pHCalibs, true);
}

// This sets the 3 calibration points for a pH sensor
// Calibration steps for pH (3 point calibration only):
//   1. Put sensor in solution and allow to stabilize for 1 minute
//   2. Input value of calibration standard to register 0x2300 (8960) (ie, run command pHCalibrationPoint(pH))
//   3. Repeat for points 2 and 3 (pH of 4.00, 6.86, and 9.18 recommended)
//   4. Read calibration status (ie, run command pHCalibrationStatus())
bool yosemitech::pHCalibrationPoint(float pH)
{
    return modbus.float32ToRegister(0x2300, pH, littleEndian);
}

// This verifies the success of a calibration
// Return values:
//   0x00 - Success
//   0x01 - Non-matching calibration standards
//   0x02 - Less than 3 points used in calibration
//   0x04 - Calibration coefficients out of range
//   0x05 - Error in sending command or receiving response+
//   The calibration status is in register 0x0E00 (3584)
byte yosemitech::pHCalibrationStatus(void)
{
    bool success = modbus.getRegisters(0x03, 0x0E00, 1);

    // Parse the response
    if (success)
    {
        return modbus.byteFromFrame(3);
    }
    else return 0x05;
}

// This sets the cap coefficients constants for a sensor
// This only applies to dissolved oxygen sensors
// The cap coefficients begin in register 0x2700 (9984)
bool yosemitech::setCapCoefficients(float K0, float K1, float K2, float K3,
                                    float K4, float K5, float K6, float K7)
{
    byte capCoeffs[32] = {0x00, };
    modbus.float32ToFrame(K0, littleEndian, capCoeffs, 0);
    modbus.float32ToFrame(K1, littleEndian, capCoeffs, 4);
    modbus.float32ToFrame(K2, littleEndian, capCoeffs, 8);
    modbus.float32ToFrame(K3, littleEndian, capCoeffs, 12);
    modbus.float32ToFrame(K4, littleEndian, capCoeffs, 16);
    modbus.float32ToFrame(K5, littleEndian, capCoeffs, 20);
    modbus.float32ToFrame(K6, littleEndian, capCoeffs, 24);
    modbus.float32ToFrame(K7, littleEndian, capCoeffs, 28);
    return modbus.setRegisters(9984, 16, capCoeffs, true);
}

// This immediately activates the cleaning brush for sensors with one.
bool yosemitech::activateBrush(void)
{
    switch (_model)
    {
        case Y4000:   // Y4000 Multiparameter sonde
        {
          byte activateBrush[9] = {_slaveID, 0x10, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                                // _slaveID, Write, Reg ???? ,0 Registers, 0byte,    CRC
          int respSize = modbus.sendCommand(activateBrush, 9);
          if (respSize == 8 && modbus.responseBuffer[0] == _slaveID) return true;
          else return false;
        }
        default:
        {
            byte activateBrush[9] = {_slaveID, 0x10, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                                  // _slaveID, Write, Reg 7168 ,0 Registers, 0byte,    CRC
            int respSize = modbus.sendCommand(activateBrush, 9);
            if (respSize == 8 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
        }
    }
}


// This sets the brush interval
// The brush interval is in register 0x3200 (12800)
bool yosemitech::setBrushInterval(uint16_t intervalMinutes)
{
    switch (_model)
    {
        case Y4000:   // Y4000 Multiparameter sonde
        {
            return modbus.uint16ToRegister(0x0E00, intervalMinutes, littleEndian, true);
        }
        default:
        {
            return modbus.uint16ToRegister(0x3200, intervalMinutes, littleEndian, true);
        }
    }
}


// This returns the brushing interval
// The brush interval is in holding register 0x3200 (12800)
uint16_t yosemitech::getBrushInterval(void)
{
    switch (_model)
    {
        case Y4000:   // Y4000 Multiparameter sonde
        {
            return modbus.int16FromRegister(0x03, 0x0E00, littleEndian);
        }
        default:
        {
            return modbus.int16FromRegister(0x03, 0x3200, littleEndian);
        }
    }
}
