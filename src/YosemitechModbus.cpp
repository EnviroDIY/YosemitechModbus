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

    return true;

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

    // Parse into a string and print that
    if (respSize == 18 && responseBuffer[0] == _slaveID)
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
        return SN;
    }
    else return "";
}

// This gets the hardware and software version of the sensor
bool yosemitech::getVersion(float hardwareVersion, float softwareVersion)
{
    byte getVersion[8] = {_slaveID, 0x03, 0x07, 0x00, 0x00, 0x02, 0x00, 0x00};
                       // _slaveID, Read,  Reg 1792 ,   2 Regs  ,    CRC
    respSize = sendCommand(getVersion, 8);

    // Parse into a string and print that
    if (respSize == 9 && responseBuffer[0] == _slaveID)
    {
        hardwareVersion = responseBuffer[3] + responseBuffer[4]/100;
        softwareVersion = responseBuffer[3] + responseBuffer[4]/100;
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
        }
        default:
        {
            byte startMeasurementR[8] = {_slaveID, 0x03, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Read,  Reg 9472 ,   0 Regs  ,    CRC
            respSize = sendCommand(startMeasurementR, 8);
            if (respSize == 5 && responseBuffer[0] == _slaveID) return true;
            else return false;
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
bool yosemitech::getValues(float value1, float value2, byte errorCode)
{    switch (_model)
    {
        case Y520:
        case Y514:
        {
            byte getValues[8] = {_slaveID, 0x03, 0x26, 0x00, 0x00, 0x05, 0x00, 0x00};
                              // _slaveID, Read,  Reg 9728 ,   5 Regs  ,    CRC
            respSize = sendCommand(getValues, 8);
        }
        default:
        {
            byte altGetValues[8] = {_slaveID, 0x03, 0x26, 0x00, 0x00, 0x04, 0x00, 0x00};
                                 // _slaveID, Read,  Reg 9728 ,   4 Regs  ,    CRC
            respSize = sendCommand(altGetValues, 8);
            if (respSize == 5 && responseBuffer[0] == _slaveID) return true;
            else return false;
        }
    }

    // Print response converted to floats
    if (respSize >= 13 && responseBuffer[0] == _slaveID)
    {
        value1 = floatFromFrame(responseBuffer, 3);
        value2 = floatFromFrame(responseBuffer, 7);
        if (respSize >= 15)  // if using "altGetValues" flags will not be sent
        {
            errorCode = responseBuffer[11];
        }
        return true;
    }
    else return false;
}

// This gets the calibration constants for the sensor
bool yosemitech::getCalibration(float K, float B)
{
    return false;
}

// This sets the calibration constants for the sensor
bool yosemitech::setCalibration(float K, float B)
{
    return false;
}

// This sets the cap coefficients constants for a sensor
// This only applies to dissolved oxygen sensors
bool yosemitech::setCapCoefficients(float K0, float K1, float K2, float K3,
                                    float K4, float K5, float K6, float K7)
{
    return false;
}

// This immediately activates the cleaning brush for sensors with one.
bool yosemitech::activateBrush(void)
{

    byte activateBrush[] = {_slaveID, 0x10, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                         // _slaveID, Write, Reg 7168 ,0 Registers, 0byte,    CRC
    return false;
}

// This sets the brush interval
bool yosemitech::setBrushInterval(int intervalMinutes)
{
    return false;
}

// This returns the brushing interval
int yosemitech::getBrushInterval(void)
{
    return 30;
}


//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

// This functions return the float from a 4-byte small-endian array beginning
// at a specific index of another array.
float yosemitech::floatFromFrame( byte indata[], int stindex)
{
    SeFrame Sefram;
    Sefram.Byte[0] = indata[stindex];
    Sefram.Byte[1] = indata[stindex + 1];
    Sefram.Byte[2] = indata[stindex + 2];
    Sefram.Byte[3] = indata[stindex + 3];
    return Sefram.Float;
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
