/*****************************************************************************
GetValues.ino

This prints basic meta-data about a sensor to the first serial port and then
begins taking measurements from the sensor.

The sensor model and address can easily be modified to use this sketch with any
Yosemitech modbus sensor.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <YosemitechModbus.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor type
yosemitechModel model = Y511;  // The sensor model number

// Define the sensor's modbus address
byte modbusAddress = 0x1A;  // The sensor's modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor *AND* RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

// Construct software serial object for Modbus
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

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

    Serial.begin(9600);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(9600);  // The modbus serial stream - Baud rate MUST be 9600.

    // Start up the sensor
    sensor.begin(model, modbusAddress, &modbusSerial, DEREPin);

    // Turn on debugging
    sensor.setDebugStream(&Serial);

    // Start up note
    Serial.print("Yosemitech ");
    Serial.print(sensor.getModel());
    Serial.print(" sensor for ");
    Serial.println(sensor.getParameter());

    // Allow the sensor and converter to warm up
    // Turbidity and DO seem to be ready in ~500ms
    // Conductivity doesn't respond until >1s
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(1100);

    // Get the sensor's hardware and software version
    Serial.println("Getting sensor version.");
    float hardwareV, softwareV;
    sensor.getVersion(hardwareV, softwareV);
    Serial.print("    Current Hardware Version: ");
    Serial.println(hardwareV);
    Serial.print("    Current Software Version: ");
    Serial.println(softwareV);

    // Get the sensor serial number
    Serial.println("Getting sensor serial number.");
    String SN = sensor.getSerialNumber();
    Serial.print("    Serial Number: ");
    Serial.println(SN);

    // Get the sensor calibration status (pH only)
    if (model == Y532)
    {
        Serial.println("Getting sensor calibration status.");
        byte status = sensor.pHCalibrationStatus();
        Serial.print("    Status: 0x0");
        Serial.println(status, HEX);
    }

    // Get the sensor's current calibration values
    if (model != Y532)
    {
        Serial.println("Getting sensor calibration equation.");
        float Kval = 0;
        float Bval = 0;
        sensor.getCalibration(Kval, Bval);
        Serial.print("    Current Calibration Equation: final = ");
        Serial.print(Kval);
        Serial.print("*raw + ");
        Serial.println(Bval);
    }

    if (model == Y511 || model == Y513 || model == Y514)
    {
        // Check the wiper timing
        Serial.println("Getting sensor cleaning interval.");
        uint16_t interval = sensor.getBrushInterval();
        Serial.print("    Sensor auto-cleaning interval: ");
        Serial.print(interval);
        Serial.println(" minutes");

        // Reset the wiper interval to 30 minutes, the default
        Serial.println("Resetting cleaning interval to 30 minutes.");
        success = sensor.setBrushInterval(5);
        if (success) Serial.println("    Reset.");
        else Serial.println("    Set interval failed!");
    }

    // Tell the sensor to start taking measurements
    Serial.println("Starting sensor measurements");
    success = sensor.startMeasurement();
    if (success) Serial.println("    Measurements started.");
    else Serial.println("    Failed to start measuring!");

    // The modbus manuals recommend the following warm-up times between starting
    // measurements and requesting values :
    //    2 s for whipered chlorophyll
    //    20 s for turbidity
    //    10 s for conductivity

    // SRGD testing - the brush immediately activates after getting power and
    // takes approximately 10-11 seconds to finish.
    // It may take up to 22 seconds to get stable values.
    Serial.println("Allowing sensor to stabilize..");
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

    if (model == Y511 || model == Y513 || model == Y514)
    {
        // We'll run the brush once in the middle of this
        Serial.println("Activating brush.");
        success = sensor.activateBrush();
        if (success) Serial.println("    Brush activated.");
        else Serial.println("    Failed to activate brush!");
    }
    if (model == Y511 || model == Y513 || model == Y514 || model == Y510)
    {
        Serial.println("Continuing to stabilize..");
        for (int i = 12; i > 0; i--)
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
    }

    Serial.print("Temp(°C)  ");
    Serial.print(sensor.getParameter());
    Serial.print("(");
    Serial.print(sensor.getUnits());
    Serial.println(")");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // send the command to get the values
    float temp = 0;
    float val = 0;
    if (model == Y532)
    {
        sensor.getValues(val);
        sensor.getTemperatureValue(temp);
    }
    else sensor.getValues(temp, val);
    Serial.print(temp);
    Serial.print("      ");
    Serial.print(val);
    Serial.println();


    // Delay between readings
    // Modbus manuals recommend the following remeasure times:
    //     2 s for chlorophyll
    //     2 s for turbidity
    //     3 s for conductivity

    // SRGD testing shows the turbidity sensor appears to be capable of taking
    // readings approximately once every 1.6 seconds, although the teperature
    // sensor can take readings much more quickly.  The same reading results
    // can be read many times from the registers between the sensor readings.
    delay(2000);
}