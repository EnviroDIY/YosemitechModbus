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
#include <Wire.h>  // For the I2C for the OLED display
#include <AMAdafruit_GFX.h>  // For the OLED display
#include <SDL_Arduino_SSD1306.h>  // For the OLED display
#include <YosemitechModbus.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor type
yosemitechModel model = Y504;  // The sensor model number

// Define the sensor's modbus address
byte modbusAddress = 0x01;  // The sensor's modbus address, or SlaveID
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

// Set up the OLED display
SDL_Arduino_SSD1306 display(-1);  // using I2C and not bothering with a reset pin

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

    // Start up the sensor
    sensor.begin(model, modbusAddress, &modbusSerial, DEREPin);

    // Start the OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);

    // Turn on debugging
    sensor.setDebugStream(&Serial);

    // Start up note
    display.println("Yosemitech ");
    display.println(sensor.getModel());
    display.display();
    display.println(sensor.getParameter());
    display.println("Sensor");
    display.display();
    delay(3000);

    // Get the sensor's hardware and software version
    display.clearDisplay();
    display.setCursor(0,0);
    float hardwareV, softwareV;
    sensor.getVersion(hardwareV, softwareV);
    display.println("Hardware Version:");
    display.println(hardwareV);
    display.println("Software Version:");
    display.println(softwareV);
    // Get the sensor serial number
    String SN = sensor.getSerialNumber();
    display.println("Serial Number:");
    display.print(SN);
    display.display();
    delay(3000);

    // Get the sensor calibration status (pH only)
    if (model == Y532)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        byte status = sensor.pHCalibrationStatus();
        display.println("Calibration Status:");
        display.print("0x0");
        display.println(status, HEX);
        display.display();
        delay(3000);
    }

    // Get the sensor's current calibration values
    if (model != Y532)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        float Kval = 0;
        float Bval = 0;
        sensor.getCalibration(Kval, Bval);
        display.println("Current Calibration Equation:");
        display.print(Kval);
        display.print("*raw + ");
        display.println(Bval);
        display.display();
        delay(3000);
    }

    if (model == Y511 || model == Y513 || model == Y514)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        // Check the wiper timing
        uint16_t interval = sensor.getBrushInterval();
        display.println("Sensor auto-cleaning interval: ");
        display.print(interval);
        display.println(" minutes");
        display.display();
        delay(3000);
    }

    // Tell the sensor to start taking measurements
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Starting sensor measurements");
    display.println();
    success = sensor.startMeasurement();
    if (success) display.println("    Measurements started.");
    else display.println("    Failed to start measuring!");
    display.display();

    // The modbus manuals recommend the following warm-up times between starting
    // measurements and requesting values :
    //    2 s for whipered chlorophyll
    //    20 s for turbidity
    //    10 s for conductivity

    // On wipered (self-cleaning) models, the brush immediately activates after
    // getting power and takes approximately 10-11 seconds to finish.  No
    // readings should be taken during this time.

    // pH returns values after ~4.5 seconds
    // Conductivity returns values after about 2.4 seconds, but is not stable
    // until ~10 seconds.
    // DO does not return values until ~8 seconds
    // Turbidity takes ~22 seconds to get stable values.
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Allowing sensor to stabilize..");
    display.display();
    for (int i = 10; i > 0; i--)
    {
        display.print(i);
        display.display();
        delay (250);
        display.print(".");
        display.display();
        delay (250);
        display.print(".");
        display.display();
        delay (250);
        display.print(".");
        display.display();
        delay (250);
    }
    display.display();

    if (model == Y511 || model == Y513 || model == Y514)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        // We'll run the brush once in the middle of this
        display.println("Activating brush.");
        display.display();
        success = sensor.activateBrush();
        if (success) display.println("    Brush activated.");
        else display.println("    Failed to activate brush!");
        display.display();
    }
    if (model == Y511 || model == Y513 || model == Y514 || model == Y510)
    {
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Continuing to stabilize..");
        display.display();
        for (int i = 12; i > 0; i--)
        {
            display.print(i);
            display.display();
            delay (250);
            display.print(".");
            display.display();
            delay (250);
            display.print(".");
            display.display();
            delay (250);
            display.print(".");
            display.display();
            delay (250);
        }
        display.println("\n");
        display.display();
    }
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);

    // send the command to get the values
    float parmValue, tempValue, thirdValue = -9999;
    sensor.getValues(parmValue, tempValue, thirdValue);
    display.println("Temp (C):");
    display.print("    ");
    display.println(tempValue);
    display.print(sensor.getParameter());
    display.print("(");
    display.print(sensor.getUnits());
    display.print("): ");
    display.print("    ");
    display.println(parmValue);
    if (model == Y532 || model == Y504)
    {
        display.println("thirdValue:");
        display.print("    ");
        display.println(thirdValue);
    }
    display.display();





    // Delay between readings
    // Modbus manuals recommend the following re-measure times:
    //     2 s for chlorophyll
    //     2 s for turbidity
    //     3 s for conductivity
    //     1 s for DO

    // The turbidity and DO sensors appear return new readings about every 1.6 seconds.
    // The pH sensor returns new readings about every 1.8 seconds.
    // The conductivity sensor only returns new readings about every 2.7 seconds.

    // The teperature sensors can take readings much more quickly.  The same results
    // can be read many times from the registers between the new sensor readings.
    delay(1700);
}
