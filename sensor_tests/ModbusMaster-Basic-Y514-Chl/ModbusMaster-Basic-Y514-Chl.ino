/*
Hybrid of these sketches
  - Basic.pde - example using ModbusMaster library
  - RS485_HalfDuplex.pde - example using ModbusMaster library to communicate
    with EPSolar LS2024B controller using a half-duplex RS485 transceiver.

By Anthony Aufdenkampe

*/

#include <ModbusMaster.h>

char sensorName[] = "Yosemitech Y514-A Chlorophyl sensor with wiper";
unsigned char getSN[8] = {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
const int     getSNBufferLen = 19; // Length of response frame, in bytes
unsigned char getSNBuffer[getSNBufferLen];   // Allocate some space for the Bytes, as n+1 element array of bytes

const long SERIAL_BAUD = 9600;  // Serial port baud rate


// instantiate ModbusMaster object
ModbusMaster node;


void setup()
{
  // use Serial (port 0); initialize Modbus communication baud rate
  Serial.begin(SERIAL_BAUD);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
  Serial1.begin(SERIAL_BAUD); //this is the Mayfly's default Xbee port (UART-1)

  // communicate with Modbus slave ID 1 over Serial1 (port 1)
  node.begin(1, Serial1);

  static uint32_t i = 0;
  uint8_t j, result;
  uint16_t data[6];
}


void loop()
{
  // Read 16 registers starting at 0x3100)
  result = node.readInputRegisters(0x3100, 16);
  if (result == node.ku8MBSuccess)
  {
    Serial.print("Vbatt: ");
    Serial.println(node.getResponseBuffer(0x04)/100.0f);
    Serial.print("Vload: ");
    Serial.println(node.getResponseBuffer(0xC0)/100.0f);
    Serial.print("Pload: ");
    Serial.println((node.getResponseBuffer(0x0D) +
                    node.getResponseBuffer(0x0E) << 16)/100.0f);
  }

  delay(1000);

}
