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
#include <AltSoftSerial.h>  // AltSoftSerial library requires use of pins 5 (Tx) and 6 (Rx) on the Mayfly
#include <YosemitechModbus.h>

const int USBserialBaud = 9600;

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor type
yosemitechModel model = Y511;  // The sensor model number

// Define the sensor's modbus address
byte oldAddress = 0x03;  // The sensor's original modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.
byte newAddress = 0x01;

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor *AND* RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
//const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
//const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

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
AltSoftSerial modbusSerial; // AltSoftSerial library requires use of pins 5 (Tx) and 6 (Rx) on the Mayfly

// Construct the Yosemitech modbus instance
yosemitech sensor;
bool success;

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{

    pinMode(PwrPin, OUTPUT);
    digitalWrite(PwrPin, HIGH);

    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(USBserialBaud);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(modbusBaud);  // The modbus serial stream - Baud rate MUST be 9600.
    modbusSerial.setTimeout(modbusFrameTimeout);

    Serial.println(F("ChangeSlaveID_AltSoftSerial.ino"));
    
    // Start up the sensor
    sensor.begin(model, oldAddress, &modbusSerial, DEREPin);

    // Turn on debugging
    sensor.setDebugStream(&Serial);

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

    Serial.println("Changing Slave ID:");
    bool success = sensor.setSlaveID(newAddress);

    if (success) Serial.println("Address change complete.");

    else  Serial.println("Address change failed!");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
// Do nothing...
void loop()
{}
