/*****************************************************************************
Modified by Anthony & Beth
From sketch from Yosemitech for
Y510-B Turbidity without wiper
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

// Define the sensor's modbus parameters
byte modbusAddress = 0x0B;  // The sensor's modbus address, or SlaveID
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

// Define arrays with the modbus commands
byte startMeasurementR[] = {modbusAddress, 0x03, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
                        // modbusAddress, Read,  Reg 9472 ,   0 Regs  ,    CRC
byte altStartMeasurementR[] = {modbusAddress, 0x03, 0x25, 0x00, 0x00, 0x01, 0x00, 0x00};
                           // modbusAddress, Read,  Reg 9472 ,   1 Reg   ,    CRC
// altStartMeasurementR is identical to startMeasurementR except that it asks for the
// value of a single register instead of asking for values in response.  Either
// can beused to start measurements.  If you use altStartMeasurementR you will
// get a longer return with the '0' value of the single register.

byte startMeasurementW[] = {modbusAddress, 0x10, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                               // Address, Write, Reg 7168 ,0 Registers, 0byte,    CRC

byte getValues[] = {modbusAddress, 0x03, 0x26, 0x00, 0x00, 0x05, 0x00, 0x00};
                  // modbusAddress, Read,  Reg 9728 ,   5 Regs  ,    CRC
byte altGetValues[] = {modbusAddress, 0x03, 0x26, 0x00, 0x00, 0x04, 0x00, 0x00};
                     // modbusAddress, Read,  Reg 9728 ,   4 Regs  ,    CRC
// altGetValues is identical to getValues except that it only asks for the 4
// registers of results, not the 5th register with the flag values.  Either can
// be used, but, obviously, you won't get the flag values with altGetValues.

byte getSN[] = {modbusAddress, 0x03, 0x09, 0x00, 0x00, 0x07, 0x00, 0x00};
             // modbusAddress, Read,  Reg 2304 ,   7 Regs  ,    CRC
byte stopMeasurement[] = {modbusAddress, 0x03, 0x2E, 0x00, 0x00, 0x00, 0x00, 0x00};
                       // modbusAddress, Read,  Reg 11776,   0 Regs  ,    CRC

// Define variables for the response;
uint32_t start;  // Timestamp for time-outs
uint32_t warmup;  // For debugging, to track stability
int bytesRead;
byte responseBuffer[20];  // This needs to be bigger than the largest response

// Define variables to hold the float values calculated from the response
float Value1, Value2;
String SN;
int errorFlag = 0;
int reserved = 0;


// ---------------------------------------------------------------------------
// Working Functions
// ---------------------------------------------------------------------------

// Define a small-endian frame as a union - that is a special class type that
// can hold only one of its non-static data members at a time, in this case,
// either 4-bytes OR a single float.
// With avr-gcc (Arduino's compiler), integer and floating point variables are
// all physically stored in memory in little-endian byte order, so this union
// is all that is needed to get the correct float value from the small-endian
// hex frames returned by Yosemitech's Modbus Sensors
union SeFrame {
  float Float;
  byte Byte[4];
};

// This functions return the float from a 4-byte small-endian array beginning
// at a specific index of another array.
float float32FromSEFrame( byte indata[], int stindex)
{
    SeFrame Sefram;  // Create an object of the small-endian class
    // Fill out the bytes
    Sefram.Byte[0] = indata[stindex];
    Sefram.Byte[1] = indata[stindex + 1];
    Sefram.Byte[2] = indata[stindex + 2];
    Sefram.Byte[3] = indata[stindex + 3];
    return Sefram.Float;  // Return the float
}

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

    // Add CRC's to all commands
    insertCRC(startMeasurementR, sizeof(startMeasurementR)/sizeof(startMeasurementR[0]));
    insertCRC(altStartMeasurementR, sizeof(altStartMeasurementR)/sizeof(altStartMeasurementR[0]));
    insertCRC(startMeasurementW, sizeof(startMeasurementW)/sizeof(startMeasurementW[0]));
    insertCRC(getValues, sizeof(getValues)/sizeof(getValues[0]));
    insertCRC(altGetValues, sizeof(altGetValues)/sizeof(altGetValues[0]));
    insertCRC(getSN, sizeof(getSN)/sizeof(getSN[0]));
    insertCRC(stopMeasurement, sizeof(stopMeasurement)/sizeof(stopMeasurement[0]));

    // Start up note
    Serial.println("Yosemitech Y510-B Optical Turbidity Sensor");

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor to warm up.");
    delay(500);

    // Send the "get serial number" command
    driverEnable();
    modbusSerial.write(getSN, sizeof(getSN)/sizeof(getSN[0]));
    modbusSerial.flush();
    // Print the raw send (for debugging)
    Serial.print("Raw Get SN Request: ");
    printFrameHex(getSN, sizeof(getSN)/sizeof(getSN[0]), &Serial);

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // Read the incoming bytes
        // 18 byte response frame for serial number, according to  the manual
        bytesRead = modbusSerial.readBytes(responseBuffer, 20);

        // Print the raw response (for debugging)
        Serial.print("Raw SN Response (");
        Serial.print(bytesRead);
        Serial.print(" bytes): ");
        printFrameHex(responseBuffer, bytesRead, &Serial);

        // Parse into a string and print that
        if (bytesRead >= 18)
        {
            int sn_len = responseBuffer[2];
            char sn_arr[sn_len] = {0,};
            int j = 0;
            for (int i = 4; i < 16; i++)
            {
                sn_arr[j] = responseBuffer[i];  // converts from "byte" or "byte" type to "char" type
                j++;
            }
            SN = String(sn_arr);
            Serial.print("Serial Number: ");
            Serial.println(SN);
        }
    }
    else
    {
        Serial.println("No response to serial number request");
    }
    emptyResponseBuffer(&modbusSerial);

    // Send the "start measurement" command
    driverEnable();
    modbusSerial.write(startMeasurementR, sizeof(startMeasurementR)/sizeof(startMeasurementR[0]));
    modbusSerial.flush();
    // Print the raw send (for debugging)
    Serial.print("Raw Start Measurement Request: ");
    printFrameHex(startMeasurementR, sizeof(startMeasurementR)/sizeof(startMeasurementR[0]), &Serial);

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // Read the incoming bytes
        // 5 byte response frame for start measurement, according to  the manual
        bytesRead = modbusSerial.readBytes(responseBuffer, 10);
        warmup = millis();

        // Print the raw response (for debugging)
        Serial.print("Raw Start Measurement Response (");
        Serial.print(bytesRead);
        Serial.print(" bytes): ");
        printFrameHex(responseBuffer, bytesRead, &Serial);
    }
    else
    {
        Serial.println("No response to Start Measurement request");
    }
    emptyResponseBuffer(&modbusSerial);

    // The modbus manuals recommend the following warm-up times between starting
    // measurements and requesting values :
    //    2 s for whipered chlorophyll
    //    20 s for turbidity
    //    10 s for conductivity

    // SRGD testing actually indicates that turbidity returns 0 until about
    // 5-6 seconds after the start measurement command.
    // It may take up to 22 seconds to get stable values.
    delay(5000);

    Serial.println("Temp(C)  Turb(NTU)  Error  Flag2  Warmup(ms)");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // send the command to get the temperature
    driverEnable();
    modbusSerial.write(getValues, sizeof(getValues)/sizeof(getValues[0]));
    modbusSerial.flush();
    // Print the raw send (for debugging)
    // Serial.print("Raw Get Value Request: ");
    // printFrameHex(getValues, sizeof(getValues)/sizeof(getValues[0]), &Serial);

    recieverEnable();
    start = millis();
    while ((modbusSerial.available() == 0) && ((millis() - start) < modbusTimeout))
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // Read the incoming bytes
        // 13 byte response frame for Cond, according to  the manual
        bytesRead = modbusSerial.readBytes(responseBuffer, 14);

        // Print the raw response (for debugging)
        // Serial.print("Raw Get Value Response (");
        // Serial.print(bytesRead);
        // Serial.print(" bytes): ");
        // printFrameHex(responseBuffer, bytesRead, &Serial);

        // Print response converted to floats
        if (bytesRead >= 13)
        {
            Value1 = float32FromSEFrame(responseBuffer, 3);
            Value2 = float32FromSEFrame(responseBuffer, 7);
            if (bytesRead >= 15)  // if using "altGetValues" flags will not be sent
            {
                errorFlag = responseBuffer[11];
                reserved = responseBuffer[12];
            }
            Serial.print(Value1, 3);
            Serial.print("     ");
            Serial.print(Value2, 3);
            Serial.print("       ");
            Serial.print(errorFlag);
            Serial.print("     ");
            Serial.print(reserved);
            Serial.print("      ");
            Serial.print(millis() - warmup);
            Serial.println();
        }
    }
    else Serial.println("  -         -           -");
    emptyResponseBuffer(&modbusSerial);

    // Delay between readings
    // Modbus manuals recommend the following remeasure times:
    //     2 s for chlorophyll
    //     2 s for turbidity
    //     3 s for conductivity

    // SRGD testing shows the turbidity sensor appears to be capable of taking
    // readings approximately once every 1.6 seconds, although the teperature
    // sensor can take readings much more quickly.  The same reading results
    // can be read many times from the registers betweeen the sensor readings.
    delay(1600);
}
