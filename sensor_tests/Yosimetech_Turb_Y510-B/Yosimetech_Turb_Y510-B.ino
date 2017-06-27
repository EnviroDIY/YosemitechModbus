/*****************************************************************************
Modified by Anthony & Beth
From sketch from YosemiTech for
Y510-B Turbidity with wiper
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>

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

// Define the sensor addresses
unsigned char modbusAddress = 0x01;
const int modbusTimeout = 1000;  // time to wait for response in ms

#include <SoftwareSerial.h>
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

// Define arrays with the modbus commands
unsigned char startMeasurement[8] = {modbusAddress, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4E, 0xC6};
                                  // Address      , Fxn , Start Addr, # Register,    CRC
                                  // modbusAddress, Read, Coil 9472 ,   0 Regs  ,    CRC
unsigned char getTempandVarX[8] = {modbusAddress, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41};
                                // Address      , Fxn , Start Addr, # Register,    CRC
                                // modbusAddress, Read, Coil 9728 ,   4 Regs  ,    CRC
unsigned char getSN[8] = {modbusAddress, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
                       // Address      , Fxn , Start Addr, # Register,    CRC
                       // modbusAddress, Read, Coil 2304 ,   7 Regs  ,    CRC
unsigned char stopMeasurement[8] = {modbusAddress, 0x03, 0x2E, 0x00, 0x00, 0x00, 0x4C, 0xE2};
                                 // Address      , Fxn , Start Addr, # Register,    CRC
                                 // modbusAddress, Read, Coil 11776,   0 Regs  ,    CRC
// Define variables for the response;
uint32_t start;  // For time-outs
uint32_t warmup;  // For time-outs
int bytesRead;
unsigned char responseBuffer[20];  // This needs to be bigger than the largest response

// Define variables to hold the float values calculated from the response
float Temperature, VarXvalue;
String SN;


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
  unsigned char Byte[4];
};

// This functions return the float from a 4-byte small-endian array beginning
// at a specific index of another array.
float Rev_float( unsigned char indata[], int stindex)
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

    Serial.begin(9600);  // Main serial port for debugging
    modbusSerial.begin(9600);

    // Allow the sensor and converter to warm up
    delay(500);

    // Send the "get serial number" command
    driverEnable();
    modbusSerial.write(getSN, 8);
    modbusSerial.flush();

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // read the incoming byte:
        bytesRead = modbusSerial.readBytes(responseBuffer, 18);

        // print the raw response (for debugging)
        // Serial.print("Raw SN Response (");
        // Serial.print(bytesRead);
        // Serial.print(" bytes):");
        // for (int i = 0; i < bytesRead; i++) Serial.print(responseBuffer[i]);
        // Serial.println();

        // Parse into a string and print that
        if (bytesRead == 18)
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
    modbusSerial.write(startMeasurement, 8);
    modbusSerial.flush();

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // read the incoming byte:
        bytesRead = modbusSerial.readBytes(responseBuffer, 18);
        warmup = millis();

        // print the raw response (for debugging)
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

    // Modbus manual (p15) recommendeds >2 second delay after Start Meaurement
    // before requesting values.  My testing actually indicates that turbidity
    // returns 0 until about 5-6 seconds after the start measurement command.
    // It may take up to 22 seconds to get stable values.
    // delay(5000);

    Serial.println("Temp(C)  Turb(NTU)  Warmup(ms)");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // send the command to get the temperature
    driverEnable();
    modbusSerial.write(getTempandVarX, 8);
    modbusSerial.flush();

    recieverEnable();
    start = millis();
    while (modbusSerial.available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}

    if (modbusSerial.available() > 0)
    {
        // read the incoming byte:
        bytesRead = modbusSerial.readBytes(responseBuffer, 18);

        // print the raw response (for debugging)
        // Serial.print("Raw Get Value Response (");
        // Serial.print(bytesRead);
        // Serial.print(" bytes):");
        // for (int i = 0; i < bytesRead; i++) Serial.print(responseBuffer[i], HEX);
        // Serial.println();

        // Print response converted to floats
        if (bytesRead == 13)
        {
            Temperature = Rev_float(responseBuffer, 3);
            VarXvalue = Rev_float(responseBuffer, 7);
            Serial.print(Temperature, 2);
            Serial.print("     ");
            Serial.print(VarXvalue, 2);
            Serial.print("       ");
            Serial.print(millis() - warmup);
            Serial.println();
        }
    }
    else Serial.println("  -         -           -");

    // Delay between readings
    // Sensor returns values no more frequently than once per second
    // delay(1000);
}
