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
    _stream = stream;
    _enablePin = enablePin;

    // Set pin mode for the enable pin
    if (_enablePin > 0) pinMode(_enablePin, OUTPUT);

    stream->setTimeout(modbusFrameTimeout);

    // Attempt to a model number based on the serial number for an unknown sensor
    if (_model == UNKNOWN) getSerialNumber();

    return true;
}

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
byte yosemitech::getSlaveID(void)
{
    byte command[8] = {0xFF, 0x03, 0x30, 0x00, 0x00, 0x01, 0x9E, 0xD4};
    respSize = sendCommand(command, 8);

    if (respSize == 7) return responseBuffer[3];
    else return 0x01;  // This is the default address
}

// This sets a new modbus slave ID
bool yosemitech::setSlaveID(byte newSlaveID)
{
    byte setSlaveID[11] = {_slaveID, 0x10, 0x30, 0x00, 0x00, 0x01, 0x02, newSlaveID, 0x00, 0x00, 0x00};
                         // Address, Write, Reg 12288, 1 Register, 2byte,newAddress, Rsvd,    CRC
    respSize = sendCommand(setSlaveID, 11);

    if (respSize == 8 && responseBuffer[0] == _slaveID)
    {
        _slaveID = newSlaveID;
        return true;
    }
    else return false;
}

// This gets the instrument serial number as a String
String yosemitech::getSerialNumber(void)
{
    byte getSN[8] = {_slaveID, 0x03, 0x09, 0x00, 0x00, 0x07, 0x00, 0x00};
                 // _slaveID, Read,  Reg 2304 ,   7 Regs  ,    CRC
    respSize = sendCommand(getSN, 8);

    // Parse into a string
    if (respSize == 19 && responseBuffer[0] == _slaveID)
    {
        int sn_len = responseBuffer[2];
        char sn_arr[sn_len] = {0,};
        int j = 0;
        for (int i = 4; i < 16; i++)
        {
            sn_arr[j] = responseBuffer[i];  // converts from "byte" or "byte" type to "char" type
            j++;
        }
        String SN = String(sn_arr);

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

        // Return the serial number
        return SN;
    }
    else return "";
}

// This gets the hardware and software version of the sensor
bool yosemitech::getVersion(float &hardwareVersion, float &softwareVersion)
{
    byte getVersion[8] = {_slaveID, 0x03, 0x07, 0x00, 0x00, 0x02, 0x00, 0x00};
                       // _slaveID, Read,  Reg 1792 ,   2 Regs  ,    CRC
    respSize = sendCommand(getVersion, 8);

    // Parse into version numbers
    // These aren't actually little endian responses.  The first byte is the
    // major version and the second byte is the minor version.
    if (respSize == 9 && responseBuffer[0] == _slaveID)
    {
        hardwareVersion = responseBuffer[3] + (float)responseBuffer[4] / 100;
        softwareVersion = responseBuffer[5] + (float)responseBuffer[6] / 100;
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
            respSize = sendCommand(startMeasurementW, 9);
            if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
            else return false;
            break;
        }
        default:
        {
            byte startMeasurementR[8] = {_slaveID, 0x03, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Read,  Reg 9472 ,   0 Regs  ,    CRC
            respSize = sendCommand(startMeasurementR, 8);
            if (respSize == 5 && responseBuffer[0] == _slaveID) return true;
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
    respSize = sendCommand(stopMeasurement, 8);
    if (respSize == 5 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This gets values back from the sensor
bool yosemitech::getValues(float &value1, float &value2, byte &errorCode)
{    switch (_model)
    {
        case Y520:
        case Y514:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x26, 0x00, 0x00, 0x05, 0x00, 0x00};
                              // _slaveID, Read,  Reg 9728 ,   5 Regs  ,    CRC
            respSize = sendCommand(getValues, 8);
            break;
        }
        case Y532:
        {
            byte getValues2[8] = {_slaveID, 0x03, 0x28, 0x00, 0x00, 0x02, 0x00, 0x00};
                               // _slaveID, Read,  Reg 10240,   2 Regs  ,    CRC
            respSize = sendCommand(getValues2, 8);
            break;
        }
        default:
        {
            byte getValues3[8] = {_slaveID, 0x03, 0x26, 0x00, 0x00, 0x04, 0x00, 0x00};
                               // _slaveID, Read,  Reg 9728 ,   4 Regs  ,    CRC
            respSize = sendCommand(getValues3, 8);
            break;
        }
    }

    // Parse the response
    // Y532 (pH)
    if (respSize == 9 && responseBuffer[0] == _slaveID)
    {
        value1 = floatFromFrame(responseBuffer, 3);
        return true;
    }
    // Y520, Y514 (Conductivity, Chlorophyll)
    if (respSize == 13 && responseBuffer[0] == _slaveID)
    {
        value1 = floatFromFrame(responseBuffer, 3);
        value2 = floatFromFrame(responseBuffer, 7);
        return true;
    }
    // Default
    if (respSize == 15 && responseBuffer[0] == _slaveID)
    {
        value1 = floatFromFrame(responseBuffer, 3);
        value2 = floatFromFrame(responseBuffer, 7);
        errorCode = responseBuffer[11];
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
{    switch (_model)
    {
        case Y532:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x12, 0x00, 0x00, 0x02, 0x00, 0x00};
                              // _slaveID, Read,  Reg 4608 ,   2 Regs  ,    CRC
            respSize = sendCommand(getValues, 8);
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
    if (respSize == 9 && responseBuffer[0] == _slaveID)
    {
        value1 = floatFromFrame(responseBuffer, 3);
        return true;
    }
    else return false;
}

// This gets the temperatures value from a sensor
// The float variable for value1 must be initialized prior to calling this function.
bool yosemitech::getTemperatureValue(float &value1)
{    switch (_model)
    {
        case Y532:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x24, 0x00, 0x00, 0x02, 0x00, 0x00};
                              // _slaveID, Read,  Reg 9216 ,   2 Regs  ,    CRC
            respSize = sendCommand(getValues, 8);

            // Parse the response
            // Y532 (pH)
            if (respSize == 9 && responseBuffer[0] == _slaveID)
            {
                value1 = floatFromFrame(responseBuffer, 3);
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
    respSize = sendCommand(getCalib, 8);

    // Parse the response
    if (respSize == 13 && responseBuffer[0] == _slaveID)
    {
        K = floatFromFrame(responseBuffer, 3);
        B = floatFromFrame(responseBuffer, 7);
        return true;
    }
    else return false;
}

// This sets the calibration constants for the sensor
// This is for all sensors EXCEPT pH
bool yosemitech::setCalibration(float K, float B)
{
    byte setCalib[17] = {_slaveID, 0x10, 0x11, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                     // _slaveID, Write, Reg 4352 ,4 Registers, 8byte,          K            ,           B           ,    CRC
    floatIntoFrame(setCalib, 7, K);
    floatIntoFrame(setCalib, 11, B);
    respSize = sendCommand(setCalib, 17);
    if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This sets the 3 calibration points for a pH sensor
// Calibration steps for pH (3 point calibration only):
//   1. Put sensor in solution and allow to stabilize for 1 minute
//   2. Input value of calibration standard (ie, run command pHCalibrationPoint(pH))
//   3. Repeat for points 2 and 3 (pH of 4.00, 6.86, and 9.18 recommended)
//   4. Read calibration status
bool yosemitech::pHCalibrationPoint(float pH)
{
    byte setpHPoint[13] = {_slaveID, 0x10, 0x23, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                        // _slaveID, Write, Reg 8960 ,2 Registers, 4byte,         pH           ,    CRC
    floatIntoFrame(setpHPoint, 7, pH);
    respSize = sendCommand(setpHPoint, 13);
    if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This verifies the success of a calibration
// Return values:
//   0x00 - Success
//   0x01 - Non-matching calibration standards
//   0x02 - Less than 3 points used in calibration
//   0x04 - Calibration coefficients out of range
//   0x05 - Error in sending command or receiving response
byte yosemitech::pHCalibrationStatus(void)
{
    byte getCalibStat[8] = {_slaveID, 0x03, 0x0E, 0x00, 0x00, 0x01, 0x00, 0x00};
                         // _slaveID, Read,  Reg 3584 ,   1 Reg   ,    CRC
    respSize = sendCommand(getCalibStat, 8);

    // Parse the response
    if (respSize == 7 && responseBuffer[0] == _slaveID)
    {
        return responseBuffer[3];
    }
    else return 0x05;
}

// This sets the cap coefficients constants for a sensor
// This only applies to dissolved oxygen sensors
bool yosemitech::setCapCoefficients(float K0, float K1, float K2, float K3,
                                    float K4, float K5, float K6, float K7)
{
    byte setCapCoef[41] = {_slaveID, 0x10, 0x27, 0x00, 0x00, 0x10, 0x20, 0x00,};
                        // _slaveID, Write, Reg 9984 ,  16 Regs  ,32byte, etc...
    floatIntoFrame(setCapCoef, 7, K0);
    floatIntoFrame(setCapCoef, 11, K1);
    floatIntoFrame(setCapCoef, 15, K2);
    floatIntoFrame(setCapCoef, 19, K3);
    floatIntoFrame(setCapCoef, 23, K4);
    floatIntoFrame(setCapCoef, 27, K5);
    floatIntoFrame(setCapCoef, 31, K6);
    floatIntoFrame(setCapCoef, 35, K7);
    respSize = sendCommand(setCapCoef, 41);
    if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This sets the calibration constants for a pH sensor
// Factory calibration values are:  K1=6.86, K2=-6.72, K3=0.04, K4=6.86, K5=-6.56, K6=-1.04
bool yosemitech::setpHCalibration(float K1, float K2, float K3,
                                  float K4, float K5, float K6)
{
    byte setpHCalib[33] = {_slaveID, 0x10, 0x29, 0x00, 0x00, 0x0C, 0x18, 0x00,};
                        // _slaveID, Write, Reg 10496,  12 Regs  ,24byte, etc...
    floatIntoFrame(setpHCalib, 7, K1);
    floatIntoFrame(setpHCalib, 11, K2);
    floatIntoFrame(setpHCalib, 15, K3);
    floatIntoFrame(setpHCalib, 19, K4);
    floatIntoFrame(setpHCalib, 23, K5);
    floatIntoFrame(setpHCalib, 27, K6);
    respSize = sendCommand(setpHCalib, 33);
    if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This immediately activates the cleaning brush for sensors with one.
bool yosemitech::activateBrush(void)
{
    byte activateBrush[9] = {_slaveID, 0x10, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                         // _slaveID, Write, Reg 7168 ,0 Registers, 0byte,    CRC
    respSize = sendCommand(activateBrush, 9);
    if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This sets the brush interval
bool yosemitech::setBrushInterval(uint16_t intervalMinutes)
{
    byte setInterval[11] = {_slaveID, 0x10, 0x32, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00};
                         // _slaveID, Write, Reg 12800,    1 reg  ,2byte,  interval ,    CRC
    SeFrame2 Sefram;
    Sefram.Int = intervalMinutes;
    setInterval[7] = Sefram.Byte[0];
    setInterval[8] = Sefram.Byte[1];
    respSize = sendCommand(setInterval, 11);
    if (respSize == 8 && responseBuffer[0] == _slaveID) return true;
    else return false;
}

// This returns the brushing interval
uint16_t yosemitech::getBrushInterval(void)
{
    byte getBrushInt[8] = {_slaveID, 0x03, 0x32, 0x00, 0x00, 0x01, 0x00, 0x00};
                        // _slaveID, Read,  Reg 12800,   1 Reg   ,    CRC
    respSize = sendCommand(getBrushInt, 8);

    // Parse the response
    if (respSize == 7 && responseBuffer[0] == _slaveID)
    {
        SeFrame2 Sefram;
        Sefram.Byte[0] = responseBuffer[3];
        Sefram.Byte[1] = responseBuffer[4];
        return Sefram.Int;
    }
    else return 0;
}


//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

// This functions return the float from a 4-byte small-endian array beginning
// at a specific index of another array.
float yosemitech::floatFromFrame(byte indata[], int stindex)
{
    SeFrame Sefram;
    Sefram.Byte[0] = indata[stindex];
    Sefram.Byte[1] = indata[stindex + 1];
    Sefram.Byte[2] = indata[stindex + 2];
    Sefram.Byte[3] = indata[stindex + 3];
    return Sefram.Float;
}
// This functions inserts a float as a 4-byte small endian array into another
// array beginning at the specified index.
void yosemitech::floatIntoFrame(byte indata[], int stindex, float value)
{
    SeFrame Sefram;
    Sefram.Float = value;
    indata[stindex] = Sefram.Byte[0];
    indata[stindex + 1] = Sefram.Byte[1];
    indata[stindex + 2] = Sefram.Byte[2];
    indata[stindex + 3] = Sefram.Byte[3];
}

// This flips the device/receive enable to DRIVER so the arduino can send text
void yosemitech::driverEnable(void)
{
    if (_enablePin > 0)
    {
        digitalWrite(_enablePin, HIGH);
        delay(8);
    }
}

// This flips the device/receive enable to RECIEVER so the sensor can send text
void yosemitech::recieverEnable(void)
{
    if (_enablePin > 0)
    {
        digitalWrite(_enablePin, LOW);
        delay(8);
    }
}

// This empties the serial buffer
void yosemitech::emptyResponseBuffer(Stream *stream)
{
    while (stream->available() > 0)
    {
        stream->read();
        delay(1);
    }
}

// Just a function to pretty-print the modbus hex frames
// This is purely for debugging
void yosemitech::printFrameHex(byte modbusFrame[], int frameLength)
{
    _debugStream->print("{");
    for (int i = 0; i < frameLength; i++)
    {
        _debugStream->print("0x");
        if (modbusFrame[i] < 16) _debugStream->print("0");
        _debugStream->print(modbusFrame[i], HEX);
        if (i < frameLength - 1) _debugStream->print(", ");
    }
    _debugStream->println("}");
}


// Calculates a Modbus RTC cyclical redudancy code (CRC)
// and adds it to the last two bytes of a frame
// From: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
// and: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
void yosemitech::insertCRC(byte modbusFrame[], int frameLength)
{
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < frameLength - 2; pos++)
    {
        crc ^= (unsigned int)modbusFrame[pos];  // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {    // If the least significant bit (LSB) is set
                crc >>= 1;                // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                          // Else least significant bit (LSB) is not set
            crc >>= 1;                    // Just shift right
        }
    }

    // Break into low and high bytes
    byte crcLow = crc & 0xFF;
    byte crcHigh = crc >> 8;

    // Append the bytes to the end of the frame
    modbusFrame[frameLength - 2] = crcLow;
    modbusFrame[frameLength - 1] = crcHigh;
}

// This sends a command to the sensor bus and listens for a response
int yosemitech::sendCommand(byte command[], int commandLength)
{
    // Add the CRC to the frame
    insertCRC(command, commandLength);

    // Send out the command
    driverEnable();
    _stream->write(command, commandLength);
    _stream->flush();
    // Print the raw send (for debugging)
    _debugStream->print("Raw Request: ");
    printFrameHex(command, commandLength);

    // Listen for a response
    recieverEnable();
    uint32_t start = millis();
    while (_stream->available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}


    if (_stream->available() > 0)
    {
        // Read the incoming bytes
        int bytesRead = _stream->readBytes(responseBuffer, 20);
        emptyResponseBuffer(_stream);

        // Print the raw response (for debugging)
        _debugStream->print("Raw Response (");
        _debugStream->print(bytesRead);
        _debugStream->print(" bytes): ");
        printFrameHex(responseBuffer, bytesRead);

        return bytesRead;
    }
    else return 0;
}
