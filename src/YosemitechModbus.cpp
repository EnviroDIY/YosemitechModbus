/*
 *YosemitechModbus.cpp
*/

#include "YosemitechModbus.h"

bool yosemitech::begin(yosemitechModel model, byte modbusSlaveID, Stream *stream, int enablePin = -1);
byte yosemitech::getSlaveID(void);
bool yosemitech::setSlaveID(byte newSlaveID);
String yosemitech::getSerialNumber(void);
bool yosemitech::startMeasurement(void);
bool yosemitech::stopMeasurement(void);
bool yosemitech::getValues(float value1, float value2, byte ErrorCode);
bool yosemitech::getVersion(float hardwareVersion, float softwareVersion);
bool yosemitech::getCalibration(float K, float B);
bool yosemitech::setCalibration(float K, float B);
bool yosemitech::setCapCoefficients(float K0, float K1, float K2, float K3,
                        float K4, float K5, float K6, float K7);
bool yosemitech::activateBrush(void);
bool yosemitech::setBrushInterval(int intervalMinutes);
int yosemitech::getBrushInterval(void);


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
    if (DEREPin > 0)
    {
        digitalWrite(DEREPin, HIGH);
        delay(8);
    }
}

// This flips the device/receive enable to RECIEVER so the sensor can send text
void yosemitech::recieverEnable(void)
{
    if (DEREPin > 0)
    {
        digitalWrite(DEREPin, LOW);
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
void yosemitech::printFrameHex(byte modbusFrame[], int frameLength, Stream *stream)
{
    stream->print("{");
    for (int i = 0; i < frameLength; i++)
    {
        stream->print("0x");
        if (modbusFrame[i] < 16) stream->print("0");
        stream->print(modbusFrame[i], HEX);
        if (i < frameLength - 1) stream->print(", ");
    }
    stream->println("}");
}


// Calculates a Modbus RTC cyclical redudancy code (CRC)
// and adds it to the last two bytes of a frame
// From: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
// and: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
void insertCRC(byte modbusFrame[], int frameLength)
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
