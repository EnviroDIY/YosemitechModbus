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
    if (_model == UNKNOWN) getSerialNumber();

    return modbus.begin(modbusSlaveID, stream, enablePin);
}
bool yosemitech::begin(yosemitechModel model, byte modbusSlaveID, Stream &stream, int enablePin)
{return begin(model, modbusSlaveID, &stream, enablePin);}

// This returns a pretty string with the model information
String yosemitech::getModel(void)
{
    switch (_model)
    {
        case Y502: {return "Y502"; break;}
        case Y504: {return "Y504"; break;}
        case Y510: {return "Y510"; break;}
        case Y511: {return "Y511"; break;}
        case Y513: {return "Y513"; break;}
        case Y514: {return "Y514"; break;}
        case Y516: {return "Y516"; break;}
        case Y520: {return "Y520"; break;}
        case Y532: {return "Y532"; break;}
        case Y533: {return "Y533"; break;}
        case Y550: {return "Y550"; break;}
        default:  {return "Unknown"; break;}
    }
}

// This returns a pretty string with the parameter measured.
String yosemitech::getParameter(void)
{
    switch (_model)
    {
        case Y502: {return "Dissolved Oxygen"; break;}
        case Y504: {return "Dissolved Oxygen"; break;}
        case Y510: {return "Turbidity"; break;}
        case Y511: {return "Turbidity"; break;}
        case Y513: {return "Blue Green Algae"; break;}
        case Y514: {return "Chlorophyll"; break;}
        case Y516: {return "Oil in Water"; break;}
        case Y520: {return "Conductivity"; break;}
        case Y532: {return "pH"; break;}
        case Y533: {return "ORP"; break;}
        case Y550: {return "UV254"; break;}
        default:  {return "Unknown"; break;}
    }
}

// This returns a pretty string with the parameter measured.
String yosemitech::getUnits(void)
{
    switch (_model)
    {
        case Y502: {return "percent"; break;}
        case Y504: {return "percent"; break;}
        case Y510: {return "NTU"; break;}
        case Y511: {return "NTU"; break;}
        case Y513: {return "µg/L"; break;}
        case Y514: {return "µg/L"; break;}
        case Y516: {return "ppb"; break;}
        case Y520: {return "mS/cm"; break;}
        case Y532: {return "pH"; break;}
        case Y533: {return "mV"; break;}
        case Y550: {return "???"; break;}
        default:  {return "Unknown"; break;}
    }
}

// This gets the modbus slave ID.  Not supported by many sensors.
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
    return modbus.setRegisters(12288, 1, dataToSend);
}

// This gets the instrument serial number as a String
// Serial number begins in holding register 0x0900 (2304) and occupies 7 registers (14 characters)
String yosemitech::getSerialNumber(void)
{
    String SN = modbus.StringFromRegister(0x03, 2304, 14);

    // Verify model and serial number match
    // Serial number to model information based on personal communication with Yosemitech
    // TODO:  Get serial numbers for the rest of the sensors
    int modelSS = SN.substring(3,5).toInt();

    // If model was unknown, assign it based on serial number
    if (_model == UNKNOWN)
    {
        if (modelSS == 1) _model = Y504;  // 01 means DO sensor
        if (modelSS == 9) _model = Y520;  // 09 means conductivity sensor
        if (modelSS == 10) _model = Y510;  // 10 means turbidity sensor
        if (modelSS == 29) _model = Y511;  // 29 means self-cleaning turbidity sensor
        if (modelSS == 48) _model = Y514;  // 48 means chlorophyll
        if (modelSS == 43) _model = Y532;  // 43 must mean pH
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
            break;
        }
        default:
        {
            byte startMeasurementR[8] = {_slaveID, 0x03, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Read,  Reg 9472 ,   0 Regs  ,    CRC
            int respSize = modbus.sendCommand(startMeasurementR, 8);
            if (respSize == 5 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
            break;
        }
    }
}

// This tells the optical sensors to stop taking measurements
bool yosemitech::stopMeasurement(void)
{
    byte stopMeasurement[8] = {_slaveID, 0x03, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00};
                           // _slaveID, Read,  Reg 11776,   0 Regs  ,    CRC
    int respSize = modbus.sendCommand(stopMeasurement, 8);
    if (respSize == 5 && modbus.responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This gets values back from the sensor
bool yosemitech::getValues(float &value1, float &value2, byte &errorCode)
{
    int respSize;
    switch (_model)
    {
        case Y520:
        case Y514:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x26, 0x00, 0x00, 0x05, 0x00, 0x00};
                              // _slaveID, Read,  Reg 9728 ,   5 Regs  ,    CRC
            respSize = modbus.sendCommand(getValues, 8);
            break;
        }
        case Y532:
        {
            byte getValues2[8] = {_slaveID, 0x03, 0x28, 0x00, 0x00, 0x02, 0x00, 0x00};
                               // _slaveID, Read,  Reg 10240,   2 Regs  ,    CRC
            respSize = modbus.sendCommand(getValues2, 8);
            break;
        }
        default:
        {
            byte getValues3[8] = {_slaveID, 0x03, 0x26, 0x00, 0x00, 0x04, 0x00, 0x00};
                               // _slaveID, Read,  Reg 9728 ,   4 Regs  ,    CRC
            respSize = modbus.sendCommand(getValues3, 8);
            break;
        }
    }

    // Parse the response
    // Y532 (pH)
    if (respSize == 9 && modbus.responseBuffer[0] == _slaveID)
    {
        value1 = modbus.float32FromFrame(littleEndian, 3);
        return true;
    }
    // Y520, Y514 (Conductivity, Chlorophyll)
    if (respSize == 13 && modbus.responseBuffer[0] == _slaveID)
    {
        value1 = modbus.float32FromFrame(littleEndian, 3);
        value2 = modbus.float32FromFrame(littleEndian, 7);
        return true;
    }
    // Default
    if (respSize == 15 && modbus.responseBuffer[0] == _slaveID)
    {
        value1 = modbus.float32FromFrame(littleEndian, 3);
        value2 = modbus.float32FromFrame(littleEndian, 7);
        errorCode = modbus.responseBuffer[11];
        return true;
    }
    else return false;
}
bool yosemitech::getValues(float &value1, float &value2)
{
    byte code;
    return getValues(value1, value2, code);
}
bool yosemitech::getValues(float &value1)
{
    float val2;
    byte code;
    return getValues(value1, val2, code);
}

// This gets raw electrical potential values back from the sensor
// This only applies to pH
bool yosemitech::getPotentialValue(float &value1)
{
    int respSize;
    switch (_model)
    {
        case Y532:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x12, 0x00, 0x00, 0x02, 0x00, 0x00};
                              // _slaveID, Read,  Reg 4608 ,   2 Regs  ,    CRC
            respSize = modbus.sendCommand(getValues, 8);
            break;
        }
        default:
        {
            return false;
            break;
        }
    }

    // Parse the response
    // Y532 (pH)
    if (respSize == 9 && modbus.responseBuffer[0] == _slaveID)
    {
        value1 = modbus.float32FromFrame(littleEndian, 3);
        return true;
    }
    else return false;
}

// This gets the temperatures value from a sensor
// The float variable for value1 must be initialized prior to calling this function.
// For the pH sensor, the temperature is in holding register 0x2400 (9216)
// For all other sensors, it is the first value
bool yosemitech::getTemperatureValue(float &value1)
{    switch (_model)
    {
        case Y532:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x24, 0x00, 0x00, 0x02, 0x00, 0x00};
                              // _slaveID, Read,  Reg 9216 ,   2 Regs  ,    CRC
            int respSize = modbus.sendCommand(getValues, 8);

            // Parse the response
            // Y532 (pH)
            if (respSize == 9 && modbus.responseBuffer[0] == _slaveID)
            {
                value1 = modbus.float32FromFrame(littleEndian, 3);
                return true;
            }
            else return false;
            break;
        }
        default:
        {
            return getValues(value1);
            break;
        }
    }
}

// This gets the calibration constants for the sensor
bool yosemitech::getCalibration(float &K, float &B)
{
    byte getCalib[8] = {_slaveID, 0x03, 0x11, 0x00, 0x00, 0x04, 0x00, 0x00};
                     // _slaveID, Read,  Reg 4352 ,   4 Regs  ,    CRC
    int respSize = modbus.sendCommand(getCalib, 8);

    // Parse the response
    if (respSize == 13 && modbus.responseBuffer[0] == _slaveID)
    {
        K = modbus.float32FromFrame(littleEndian, 3);
        B = modbus.float32FromFrame(littleEndian, 7);
        return true;
    }
    else return false;
}

// This sets the calibration constants for the sensor
// This is for all sensors EXCEPT pH
// The k value begins in register 0x1100 (4352) and the B value two registers later
bool yosemitech::setCalibration(float K, float B)
{
    bool success;
    success &= modbus.float32ToRegister(4352, K, littleEndian);
    success &= modbus.float32ToRegister(4354, B, littleEndian);
    return success;
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
    return modbus.setRegisters(9984, 16, capCoeffs);
}

// This sets the calibration constants for a pH sensor
// Factory calibration values are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86, K5=-6.56, K6=-1.04
// The calibration constants begin at register 0x2900 (10496)
bool yosemitech::setpHCalibration(float K1, float K2, float K3,
                                  float K4, float K5, float K6)
{
    byte pHCalibs[24] = {0x00,};
    modbus.float32ToFrame(K1, littleEndian, pHCalibs, 0);
    modbus.float32ToFrame(K2, littleEndian, pHCalibs, 4);
    modbus.float32ToFrame(K3, littleEndian, pHCalibs, 8);
    modbus.float32ToFrame(K4, littleEndian, pHCalibs, 12);
    modbus.float32ToFrame(K5, littleEndian, pHCalibs, 16);
    modbus.float32ToFrame(K6, littleEndian, pHCalibs, 20);
    return modbus.setRegisters(10496, 12, pHCalibs);
}

// This immediately activates the cleaning brush for sensors with one.
bool yosemitech::activateBrush(void)
{
    byte activateBrush[9] = {_slaveID, 0x10, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                         // _slaveID, Write, Reg 7168 ,0 Registers, 0byte,    CRC
    int respSize = modbus.sendCommand(activateBrush, 9);
    if (respSize == 8 && modbus.responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This sets the brush interval
// The brush interval is in register 0x3200 (12800)
bool yosemitech::setBrushInterval(uint16_t intervalMinutes)
{
    return modbus.uint16ToRegister(12800, intervalMinutes, littleEndian);
}

// This returns the brushing interval
// The brush interval is in holding register 0x3200 (12800)
uint16_t yosemitech::getBrushInterval(void)
{
    return modbus.int16FromRegister(0x03, 12800, littleEndian);
}
