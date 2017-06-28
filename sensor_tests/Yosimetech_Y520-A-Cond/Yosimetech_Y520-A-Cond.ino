/*****************************************************************************
Modified by Anthony & Beth
From sketch from YosemiTech for
Y520 CT conductivity sensor
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
byte modbusAddress = 0x01;  // The sensor's modbus address, or SlaveID
const int modbusTimeout = 500;  // The time to wait for response after a command (in ms)
const int modbusBaud = 9600;  // The baudrate for the modbus connection
const int modbusFrameTimeout = 3;  // the time to wait between characters within a frame (in ms)
// The modbus protocol defines that there can be no more than 1.5 characters
// of silence between characters in a frame and any space over 3.5 characters
// defines a new frame.
// At 9600 baud with 1 start bit, no parity and 1 stop bit 1 character takes ~1.04ms
// So the readBytes() command should time out within 3ms

// Construct software serial object for Modbus
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

// Define arrays with the modbus commands
byte startMeasurement[] = {modbusAddress, 0x10, 0x1C, 0x00, 0x00, 0x00, 0x00, 0xD8, 0x92};
                        // Address      , Fxn , Start Addr, # Register,    CRC
                        // modbusAddress, Read, Coil 9472 ,   0 Regs  ,    CRC
byte getResults[] = {modbusAddress, 0x03, 0x26, 0x00, 0x00, 0x05, 0x8E, 0x81};
                  // Address      , Fxn , Start Addr, # Register,    CRC
                  // modbusAddress, Read, Coil 9728 ,   5 Regs  ,    CRC
byte altGetResults[] = {modbusAddress, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41};
                     // Address      , Fxn , Start Addr, # Register,    CRC
                     // modbusAddress, Read, Coil 9728 ,   4 Regs  ,    CRC
byte getSN[] = {modbusAddress, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
             // Address      , Fxn , Start Addr, # Register,    CRC
             // modbusAddress, Read, Coil 2304 ,   7 Regs  ,    CRC
byte stopMeasurement[] = {modbusAddress, 0x03, 0x2E, 0x00, 0x00, 0x00, 0x4C, 0xE2};
                       // Address      , Fxn , Start Addr, # Register,    CRC
                       // modbusAddress, Read, Coil 11776,   0 Regs  ,    CRC

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
// hex frames returned by YosemiTech's Modbus Sensors
union SeFrame {
  float Float;
  byte Byte[4];
};

// This functions return the float from a 4-byte small-endian array beginning
// at a specific index of another array.
float floatFromFrame( byte indata[], int stindex)
{
    SeFrame Sefram;
    Sefram.Byte[0] = indata[stindex];
    Sefram.Byte[1] = indata[stindex + 1];
    Sefram.Byte[2] = indata[stindex + 2];
    Sefram.Byte[3] = indata[stindex + 3];
  return Sefram.Float;
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
    delay(5000);

    // Send the "get serial number" command
    driverEnable();
    modbusSerial.write(getSN, sizeof(getSN)/sizeof(getSN[0]));
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

        // Print the raw response (for debugging)
        // Serial.print("Raw SN Response (");
        // Serial.print(bytesRead);
        // Serial.print(" bytes):");
        // for (int i = 0; i < bytesRead; i++) Serial.print(responseBuffer[i]);
        // Serial.println();

        // Parse into a string and print that
        if (bytesRead >= 18)
        {
            int sn_len = responseBuffer[2];
            char sn_arr[sn_len] = {0,};
            int j = 0;
            for (int i = 4; i < 16; i++)
            {
                sn_arr[j] = responseBuffer[i];
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

    // Send the "start measurement" command
    driverEnable();
    modbusSerial.write(startMeasurement, sizeof(startMeasurement)/sizeof(startMeasurement[0]));
    modbusSerial.flush();

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // Read the incoming bytes
        // 8 byte response frame for start measurement, according to  the manual
        bytesRead = modbusSerial.readBytes(responseBuffer, 10);
        warmup = millis();

        // Print the raw response (for debugging)
        Serial.print("Raw Start Measurement Response (");
        Serial.print(bytesRead);
        Serial.print(" bytes):");
        for (int i = 0; i < bytesRead; i++) Serial.print(responseBuffer[i], HEX);
        Serial.println();
    }
    else
    {
        Serial.println("No response to Start Measurement request");
    }

    // The modbus manuals recommend the following warm-up times between starting
    // measurements and requesting values :
    //    2 s for whipered chlorophyll
    //    20 s for turbidity
    //    10 s for conductivity

    // delay(7000);

    Serial.println("Temp(C)  Cond(mS/cm)  Error  Flag2  Warmup(ms)");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // send the command to get the temperature
    driverEnable();
    modbusSerial.write(getResults, sizeof(getResults)/sizeof(getResults[0]));
    modbusSerial.flush();

    recieverEnable();
    start = millis();
    while ((modbusSerial.available() == 0) && ((millis() - start) < modbusTimeout))
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // Read the incoming bytes
        // 15 byte response frame for Cond, according to  the manual
        bytesRead = modbusSerial.readBytes(responseBuffer, 17);

        // Print the raw response (for debugging)
        Serial.print("Raw Get Value Response (");
        Serial.print(bytesRead);
        Serial.print(" bytes):");
        for (int i = 0; i < bytesRead; i++) Serial.print(responseBuffer[i], HEX);
        Serial.println();

        // Print response converted to floats
        if (bytesRead >= 13)
        {
            Value1 = floatFromFrame(responseBuffer, 3);
            Value2 = floatFromFrame(responseBuffer, 7);
            if (bytesRead >= 15)  // if using "altGetResults" flags will not be sent
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

    // Delay between readings
    // Modbus manuals recommend the following remeasure times:
    //     2 s for chlorophyll
    //     2 s for turbidity
    //     3 s for conductivity

    delay(500);
}
