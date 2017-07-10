/*****************************************************************************
Yosemitech_ChangeSlaveID.ino

This sends a change address command to a Yosemitech sensor.

You MUST know the old address of this sensor or it will not work.  The (poor)
implentation of modbus for the Yosemitech sensors does not support responses
to broadcasts at address 0x00.  (Official modbus specifications require that
all slaves accept and comply with writing functions broadcast to address 0.)
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <SoftwareSerial.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor AND RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

// Define the old and new sensor addresses
byte oldAddress = 0x01;
byte newAddress = 0x1A;


// Define the sensor's modbus parameters
const int modbusTimeout = 500;  // The time to wait for response after a command (in ms)
const int modbusBaud = 9600;  // The baudrate for the modbus connection
const int modbusFrameTimeout = 4;  // the time to wait between characters within a frame (in ms)
// The modbus protocol defines that there can be no more than 1.5 characters
// of silence between characters in a frame and any space over 3.5 characters
// defines a new frame.
// At 9600 baud with 1 start bit, no parity and 1 stop bit 1 character takes ~1.04ms
// So the readBytes() command should time out within 3ms

// Construct software serial object for Modbus
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

// Define variables for the response;
uint32_t start;  // Timestamp for time-outs
int bytesRead;
byte responseBuffer[20];  // This needs to be bigger than the largest response

// Define variables to hold the float values calculated from the response
String SN;


// ---------------------------------------------------------------------------
// Working Functions
// ---------------------------------------------------------------------------
// This flips the device/receive enable to DRIVER so the arduino can send text
void driverEnable(void)
{
    if (DEREPin > 0)
    {
        digitalWrite(DEREPin, HIGH);
        delay(8);
    }
}

// This flips the device/receive enable to RECIEVER so the sensor can send text
void recieverEnable(void)
{
    if (DEREPin > 0)
    {
        digitalWrite(DEREPin, LOW);
        delay(8);
    }
}

// This empties the serial buffer
void emptyResponseBuffer(Stream *stream)
{
    while (stream->available() > 0)
    {
        stream->read();
        delay(1);
    }
}

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

// Just a function to pretty-print the modbus hex frames
void printFrameHex(byte modbusFrame[], int frameLength, Stream *stream)
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

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{

    pinMode(PwrPin, OUTPUT);
    digitalWrite(PwrPin, HIGH);

    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(9600);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(modbusBaud);
    modbusSerial.setTimeout(modbusFrameTimeout);

    // Allow the sensor and converter to warm up
    Serial.println(F("Allowing sensor and adapter to warm up"));
    for (int i = 10; i > 0; i--)
    {
        Serial.print(i);
        delay (250);
        Serial.print(".");
        delay (250);
        Serial.print(".");
        delay (250);
        Serial.print(".");
        delay (250);
    }
    Serial.println("\n");

    // Set up the setSlaveID command
    byte setSlaveID[] = {oldAddress, 0x10, 0x30, 0x00, 0x00, 0x01, 0x02, newAddress, 0x00, 0x00, 0x00};
                        // Address, Write,  Reg 12288, 1 Register, 2byte,newAddress, Rsvd,    CRC
    insertCRC(setSlaveID, sizeof(setSlaveID)/sizeof(setSlaveID[0]));

    // Send the set slave ID command
    driverEnable();
    modbusSerial.write(setSlaveID, sizeof(setSlaveID)/sizeof(setSlaveID[0]));
    modbusSerial.flush();
    // Print the raw send (for debugging)
    Serial.print("Set Slave ID Request: ");
    printFrameHex(setSlaveID, sizeof(setSlaveID)/sizeof(setSlaveID[0]), &Serial);

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // Read the incoming bytes
        // 8 byte response frame for start measurement, according to  the manual
        bytesRead = modbusSerial.readBytes(responseBuffer, 10);

        // Print the raw response (for debugging)
        Serial.print("Set Slave ID Response (");
        Serial.print(bytesRead);
        Serial.print(" bytes): ");
        printFrameHex(responseBuffer, bytesRead, &Serial);
    }
    else
    {
        Serial.println("No response to Set Slave ID request");
    }
    emptyResponseBuffer(&modbusSerial);

    Serial.println("Address change complete.");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{}
