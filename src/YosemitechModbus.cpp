/*
 *YosemitechModbus.cpp
*/

#include "YosemitechModbus.h"


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

// From: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
// and: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
uint16_t yosemitech::ModRTU_CRC(byte modbusFrame[], int frameLength)
{
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < frameLength; pos++)
  {
  crc ^= (unsigned int)modbusFrame[pos];  // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }
  // Reverse byte order so crcLo byte is first & crcHi byte is last
  uint16_t crc2 = crc >> 8;
  crc = (crc << 8) | crc2;
  crc &= 0xFFFF;
  return crc;
}
