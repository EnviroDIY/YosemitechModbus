/*****************************************************************************
Yosemitech_GetSlaveID.ino

This scans through all possible addresses and asks for the standard Yosemitech
serial number response at each address to idenitfy the sensors.

The scan will take several minutes to complete.  (Sorry, please be patient!)

I recommend that if you really don't know your device's address, you only attach
a single device to the bus while running this scan because if multiple devices
on the bus have the same address their responses will overlap each-other.

This uses the serial number request because not all of the Yosemitech sensors
will respond to a Get Slave Device ID command sent to address 0xFF and none
of the sensors support modbus function 17 (report slave ID).

The sensors also do not seem to respond to any register requests except those
exactly listed in the manuals, that is, I cannot even send a generic pool for
register 1 because the sensors will not respond.  The get serial number command
seems to be one of few all support identically.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
// #include <SoftwareSerial.h>
#include <AltSoftSerial.h>
// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor *AND* RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
// const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
// const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

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
// SoftwareSerial modbusSerial(SSRxPin, SSTxPin);
AltSoftSerial modbusSerial;


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

String parseSN(byte modbusFrame[])
{
    int sn_len = responseBuffer[2];
    char sn_arr[sn_len] = {0,};
    int j = 0;
    for (int i = 4; i < 16; i++)
    {
        sn_arr[j] = responseBuffer[i];
        j++;
    }
    String SN = String(sn_arr);
    return SN;
}

void scanSNs(void)
{
    Serial.println(F("Scanning for Yosemitech modbus sensors...."));
    Serial.println(F("------------------------------------------"));
    Serial.println(F("Modbus Address ------ Sensor Serial Number"));

    int numFound = 0;
    for (uint8_t addrTest = 1; addrTest <= 247; addrTest++)
    {
        byte getSN[] = {addrTest, 0x03, 0x09, 0x00, 0x00, 0x07, 0x00, 0x00}; // for all except Y4000 Sonde
//        byte getSN[] = {addrTest, 0x03, 0x14, 0x00, 0x00, 0x07, 0x00, 0x00}; // for Y4000 Sonde
        insertCRC(getSN, sizeof(getSN)/sizeof(getSN[0]));

        // Send the "get serial number" command
        driverEnable();
        modbusSerial.write(getSN, sizeof(getSN)/sizeof(getSN[0]));
        printFrameHex(getSN, sizeof(getSN)/sizeof(getSN[0]), &Serial);
        modbusSerial.flush();

        recieverEnable();
        start = millis();
        while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
        { delay(1);}

        if (modbusSerial.available() > 0)
        {
            // Read the incoming bytes
            // 18 byte response frame for serial number, according to  the manual
            bytesRead = modbusSerial.readBytes(responseBuffer, 20);

            // Parse into a string and print that
            if (bytesRead >= 18)
            {
                SN = parseSN(responseBuffer);
                numFound++;
                Serial.print(F("     0x"));
                if (addrTest < 16) Serial.print(F("0"));
                Serial.print(addrTest, HEX);
                Serial.print(F("      ------    "));
                Serial.println(SN);
            }
            else  // if recieved a response, but less than 18 bytes
            {
                Serial.print(F("     0x"));
                if (addrTest < 16) Serial.print(F("0"));
                Serial.print(addrTest, HEX);
                Serial.print(F("      ------    "));
                Serial.println(F("????????????"));
            }
        }
        emptyResponseBuffer(&modbusSerial);
        // A short delay between sensors helps.
        delay(500);
    }
    Serial.println(F("------------------------------------------"));
    Serial.println(F("Scan complete."));

    if (numFound == 0) Serial.println(F("XXX  --  NO SENSORS FOUND  --  XXX"));
}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{

    pinMode(PwrPin, OUTPUT);
    digitalWrite(PwrPin, HIGH);

    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(modbusBaud);
    modbusSerial.setTimeout(modbusFrameTimeout);

    Serial.println(F("GetSlaveID_AltSoftSerial.ino"));

    // Allow the sensor and converter to warm up
    Serial.println("\n");
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

    Serial.println("Scan takes several minutes, please be patient!");
    Serial.println("\n");

    scanSNs();
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{}
