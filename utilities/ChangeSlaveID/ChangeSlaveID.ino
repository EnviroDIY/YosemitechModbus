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
// #include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <YosemitechModbus.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor type
yosemitechModel model = UNKNOWN;  // The sensor model number

// Define the sensor's modbus address
byte oldAddress = 0x01;  // The sensor's original modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.
byte newAddress = 0x1A;

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor *AND* RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
// const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
// const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

// Construct software serial object for Modbus
// SoftwareSerial modbusSerial(SSRxPin, SSTxPin);
AltSoftSerial modbusSerial;

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

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(9600);  // The modbus serial stream - Baud rate MUST be 9600.

    Serial.println(F("ChangeSlaveID_AltSoftSerial.ino"));

    // Start up the sensor
    sensor.begin(model, oldAddress, &modbusSerial, DEREPin);

    // Turn on debugging
    sensor.setDebugStream(&Serial);

    // Start up note
    Serial.print("Yosemitech ");
    Serial.print(sensor.getModel());
    Serial.print(" sensor for ");
    Serial.print(sensor.getParameter());
    Serial.print(", Serial Number ");
    Serial.println(sensor.getSerialNumber());

    // Allow the sensor and converter to warm up
    // DO responds within 275-300ms;
    // Turbidity and pH within 500ms
    // Conductivity doesn't respond until 1.15-1.2s
    Serial.println("Waiting for sensor and adapter to be ready.");
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
