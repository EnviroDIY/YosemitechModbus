/** =========================================================================
 * @example{lineno} DisplayValues.ino
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 * @copyright Stroud Water Research Center
 * @license This example is published under the BSD-3 license.
 *
 * @brief This polls the sensor for some of its setup information, and then polls the
 * sensor every several seconds for values, and displays the values on an SSD1306 128x64
 * LCD board.
 *
 * The sensor model and address can easily be modified to use this sketch with any
 * Yosemitech modbus sensor.
 *
 * @m_examplenavigation{example_display_values,}
 * @m_footernavigation
 * ======================================================================= */

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>                 // For the I2C for the OLED display
#include <AMAdafruit_GFX.h>       // For the OLED display
#include <SDL_Arduino_SSD1306.h>  // For the OLED display
#include <YosemitechModbus.h>

#if defined __AVR__
#include <AltSoftSerial.h>
// #include <SoftwareSerial.h>
#endif

#if defined ESP8266
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#endif

// Turn on debugging outputs (i.e. raw Modbus requests & responses)
// by uncommenting next line (i.e. `#define DEBUG`)
// #define DEBUG


// ==========================================================================
//  Sensor Settings
// ==========================================================================
// Define the sensor type
yosemitechModel model = Y504;  // The sensor model number

// Define the sensor's modbus address, or SlaveID
// NOTE: YosemiTech Windows software presents SlaveID as an integer (decimal),
// whereas EnviroDIY and most other modbus systems present it in hexadecimal form.
// Use an online "HEX to DEC Converter".
byte modbusAddress =
    0x01;  // Yosemitech ships sensors with a default ID of 0x01.

// Sensor Timing
// Edit these to explore
#define WARM_UP_TIME \
    1000  // milliseconds for sensor to respond to commands.
          // DO responds within 275-300ms;
          // Turbidity and pH within 500ms
          // Conductivity doesn't respond until 1.15-1.2s

#define BRUSH_TIME 10000  // milliseconds for readings to stabilize.
           // On wipered (self-cleaning) models, the brush immediately activates after
           // getting power and takes approximately 10-15 seconds to finish.
           // Turbidity takes 10-11 s
           // Ammonium takes 15 s
// No readings should be taken during this time.

#define STABILIZATION_TIME 4000  // milliseconds for readings to stabilize.
// The modbus manuals recommend the following stabilization times between starting
// measurements and requesting values (times include brushing time):
//  2 s for wipered chlorophyll
// 20 s for turbidity, including 11 s to complete a brush cycle
// 10 s for conductivity
//  2 s for COD
// 20 s for Ammonium, including 15 s to complete a brush cycle

// pH returns values after ~4.5 seconds
// Conductivity returns values after about 2.4 seconds, but is not stable
// until ~10 seconds.
// DO does not return values until ~8 seconds
// Turbidity takes ~22 seconds to get stable values.
// Y700 Water Level takes 4 s to get stability <1 mm, but 12 s for <0.1 mm

#define MEASUREMENT_TIME 1000  // milliseconds to complete a measurement.
// Modbus manuals recommend the following re-measure times:
//     2 s for chlorophyll
//     2 s for turbidity
//     3 s for conductivity
//     1 s for DO
//     2 s for COD
//     2 s for Ammonium
// The turbidity and DO sensors appear return new readings about every 1.6 seconds.
// The pH sensor returns new readings about every 1.8 seconds.
// The conductivity sensor only returns new readings about every 2.7 seconds.
// The temperature sensors can take readings much more quickly.  The same results
// can be read many times from the registers between the new sensor readings.


// ==========================================================================
//  Data Logging Options
// ==========================================================================
const int32_t serialBaud = 115200;  // Baud rate for serial monitor

// Define pin number variables
const int sensorPwrPin  = 11;  // The pin sending power to the sensor
const int adapterPwrPin = 22;  // The pin sending power to the RS485 adapter
const int DEREPin       = -1;  // The pin controlling Receive Enable and Driver Enable
                               // on the RS485 adapter, if applicable (else, -1)
                               // Setting HIGH enables the driver (arduino) to send text
                               // Setting LOW enables the receiver (sensor) to send text
// Pins for `SoftwareSerial` only. Not for `AltSoftSerial`, which uses fixed pins.
const int SSRxPin = 13;  // Receive pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 14;  // Send pin for software serial (Tx on RS485 adapter)

// Construct software serial object for Modbus
#if defined __AVR__
                         // SoftwareSerial modbusSerial(SSRxPin, SSTxPin);
AltSoftSerial modbusSerial;
#elif defined ESP8266
SoftwareSerial modbusSerial;
#elif defined(NRF52832_FEATHER) || defined(ARDUINO_NRF52840_FEATHER)
#include <Adafruit_TinyUSB.h>
HardwareSerial& modbusSerial = Serial1;
#elif !defined(NO_GLOBAL_SERIAL1)
HardwareSerial& modbusSerial = Serial1;
#else
HardwareSerial& modbusSerial = Serial;
#endif

// Construct the Yosemitech modbus instance
yosemitech sensor;
bool       success;

// Set up the OLED display
SDL_Arduino_SSD1306 display(-1);  // using I2C and not bothering with a reset pin

// ==========================================================================
// Working Functions
// ==========================================================================
// A function for pretty-printing the Modbuss Address, from ModularSensors
String sensorLocation(byte _modbusAddress) {
    String sensorLocation = F("0x");
    if (_modbusAddress < 0x10) { sensorLocation += "0"; }
    sensorLocation += String(_modbusAddress, HEX);
    return sensorLocation;
}


// ==========================================================================
//  Arduino Setup Function
// ==========================================================================
void setup() {
    if (sensorPwrPin > 0) {
        pinMode(sensorPwrPin, OUTPUT);
        digitalWrite(sensorPwrPin, HIGH);
    }
    if (adapterPwrPin > 0) {
        pinMode(adapterPwrPin, OUTPUT);
        digitalWrite(adapterPwrPin, HIGH);
    }

    if (DEREPin > 0) { pinMode(DEREPin, OUTPUT); }

    if (DEREPin > 0) { pinMode(DEREPin, OUTPUT); }

    Serial.begin(serialBaud);  // Main serial port for debugging via USB Serial Monitor

// Setup Modbus serial stream
#if defined ESP8266
    modbusSerial.begin(9600, SWSERIAL_8N1, SSRxPin, SSTxPin, false,
                       256);  // The modbus serial stream - Baud rate MUST be 9600.
#else
    modbusSerial.begin(9600);  // The modbus serial stream - Baud rate MUST be 9600.
#endif

    // Start up the sensor
    sensor.begin(model, modbusAddress, &modbusSerial, DEREPin);

    // Start the OLED
    // cspell:ignore SSD1306_SWITCHCAPVCC
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

// Turn on debugging
#ifdef DEBUG
    sensor.setDebugStream(&Serial);
#endif

    // Start up note
    Serial.print("\nYosemitech ");
    Serial.print(sensor.getModel());
    Serial.print(" sensor for ");
    Serial.println(sensor.getParameter());

    display.println("Yosemitech ");
    display.println(sensor.getModel());
    display.display();
    display.println(sensor.getParameter());
    display.println("Sensor");
    display.display();
    delay(3000);

    // Allow the sensor and converter to warm up
    Serial.println("\nWaiting for sensor and adapter to be ready.");
    Serial.print("    Warm up time (ms): ");
    Serial.println(WARM_UP_TIME);
    delay(WARM_UP_TIME);

    // Confirm Modbus Address
    Serial.println("\nSelected modbus address:");
    Serial.print("    integer: ");
    Serial.print(modbusAddress, DEC);
    Serial.print(", hexidecimal: ");
    Serial.println(sensorLocation(modbusAddress));

    Serial.println("Discovered modbus address.");
    Serial.print("    integer: ");
    byte id = sensor.getSlaveID();
    Serial.print(id, DEC);
    Serial.print(", hexidecimal: ");
    // Serial.print(id, HEX);
    Serial.println(sensorLocation(id));

    if (id != modbusAddress) {
        Serial.print("Updating sensor modbus address to: ");
        modbusAddress = id;
        Serial.println(sensorLocation(modbusAddress));
        Serial.println();
        // Restart the sensor
        sensor.begin(model, modbusAddress, &modbusSerial, DEREPin);
        delay(1500);
    };

    // Get the sensor serial number
    Serial.println("\nGetting sensor serial number.");
    String SN = sensor.getSerialNumber();
    Serial.print("    Serial Number: ");
    Serial.println(SN);

    // Display SN on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Serial Number:");
    display.print(SN);
    display.display();

    // Get the sensor's hardware and software version
    Serial.println("Getting sensor version numbers.");
    float hardwareV, softwareV;
    sensor.getVersion(hardwareV, softwareV);
    Serial.print("    Current Hardware Version: ");
    Serial.println(hardwareV);
    Serial.print("    Current Software Version: ");
    Serial.println(softwareV);

    // Display versions on OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Hardware Version:");
    display.println(hardwareV);
    display.println("Software Version:");
    display.println(softwareV);


    // Get/set the sensor brush status (for sensors with brushes).
    // NOTE: Not implemented for Y4000
    if (model == Y511 || model == Y513 || model == Y514 || model == Y551 ||
        model == Y560) {
        // Check the wiper timing
        Serial.println("\nGetting sensor cleaning interval.");
        uint16_t interval = sensor.getBrushInterval();
        Serial.print("    Sensor auto-cleaning interval: ");
        Serial.print(interval);
        Serial.println(" minutes");

        // Display wiper timing on OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Sensor auto-cleaning interval: ");
        display.print(interval);
        display.println(" minutes");
        display.display();
        delay(3000);
    }

    // Activate the brush, for sensors that have a brush.
    if (model == Y511 || model == Y513 || model == Y514 || model == Y551 ||
        model == Y560 || model == Y4000)
    // Y4000 activates brush when powered on
    {
        // We'll run the brush once in the middle of this
        Serial.println("Activating brush.");
        success = sensor.activateBrush();
        if (success)
            Serial.println("    Brush activated.");
        else
            Serial.println("    Failed to activate brush!");
    }
    // Additional stabilization time for sensors that have a brush to complete a brush
    // cycle.
    if (model == Y511 || model == Y513 || model == Y514 || model == Y551 ||
        model == Y560 || model == Y4000 || model == Y510) {
        Serial.println("Waiting to complete brushing cycle..");
        Serial.print("    Brush time (ms): ");
        Serial.println(BRUSH_TIME);
        for (int i = (BRUSH_TIME + 500) / 1000; i > 0; i--)  // +500 to round up
        {
            Serial.print(i);
            delay(250);
            Serial.print(".");
            delay(250);
            Serial.print(".");
            delay(250);
            Serial.print(".");
            delay(250);
        }
        Serial.println("\n");
    }

    // Tell the sensor to start taking measurements
    Serial.println("Starting sensor measurements");
    success = sensor.startMeasurement();
    if (success) {
        Serial.println("    Measurements started.");
    } else {
        Serial.println("    Failed to start measuring!");
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Starting sensor measurements");
    display.println();
    if (success) {
        display.println("    Measurements started.");
    } else {
        display.println("    Failed to start measuring!");
    }
    display.display();

    Serial.println("Waiting for sensor to stabilize..");
    Serial.print("    Stabilization time (ms): ");
    Serial.println(STABILIZATION_TIME);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Allowing sensor to stabilize..");
    display.display();

    for (int i = (STABILIZATION_TIME + 500) / 1000; i > 0; i--)  // +500 to round up
    {
        Serial.print(i);
        delay(250);
        Serial.print(".");
        delay(250);
        Serial.print(".");
        delay(250);
        Serial.print(".");
        delay(250);
    }
    Serial.println("\n");


    // Print table headers
    switch (model) {
        case Y4000: {
            Serial.print("Time(ms) ");
            Serial.println(sensor.getParameter());
            // "DO,   Turb, Cond,  pH,   Temp, ORP,  Chl,  BGA"
            Serial.print("ms       ");
            Serial.println(sensor.getUnits());
            // "mg/L, NTU,  mS/cm, pH,   °C,   mV,   µg/L, µg/L"
            break;
        }
        default: {
            Serial.print("Time(ms) ");
            Serial.print("Temp(°C)  ");
            Serial.print(sensor.getParameter());
            Serial.print("(");
            Serial.print(sensor.getUnits());
            Serial.print(")");
            if (model == Y532 || model == Y504) { Serial.print("    Value"); }
            if (model == Y551) { Serial.print("    Turbidity (NTU)"); }
            if (model == Y560) { Serial.print("    pH"); }
            Serial.println();
        }
    }
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop() {
    // send the command to get the values
    switch (model) {
        case Y4000: {
            float DOmgL, Turbidity, Cond, pH, Temp, ORP, Chlorophyll, BGA = -9999;
            // byte errorCode = 0xFF;  // Error!

            sensor.getValues(DOmgL, Turbidity, Cond, pH, Temp, ORP, Chlorophyll, BGA);

            Serial.print(millis());
            Serial.print("    ");
            Serial.print(DOmgL, 4);
            Serial.print("  ");
            Serial.print(Turbidity, 4);
            Serial.print("  ");
            Serial.print(Cond, 4);
            Serial.print("  ");
            Serial.print(pH, 4);
            Serial.print("  ");
            Serial.print(Temp, 4);
            Serial.print("  ");
            Serial.print(ORP, 4);
            Serial.print("  ");
            Serial.print(Chlorophyll, 4);
            Serial.print("  ");
            Serial.print(BGA, 4);
            Serial.println();
            break;
        }
        default: {
            float parmValue, tempValue, thirdValue = -9999;
            sensor.getValues(parmValue, tempValue, thirdValue);

            Serial.print(millis());
            Serial.print("     ");
            Serial.print(tempValue, 4);
            Serial.print("   ");
            Serial.print(parmValue, 4);
            if (model == Y532 || model == Y504 || model == Y551 || model == Y560) {
                Serial.print("          ");
                Serial.print(thirdValue, 4);
            }
            Serial.println();

            display.clearDisplay();
            display.setCursor(0, 0);
            display.setTextSize(2);
            display.println("Temp (C):");
            display.print("    ");
            display.println(tempValue);
            display.print(sensor.getParameter());
            display.print("(");
            display.print(sensor.getUnits());
            display.print("): ");
            display.print("    ");
            display.println(parmValue);
            if (model == Y532 || model == Y504) {
                display.println("thirdValue:");
                display.print("    ");
                display.println(thirdValue);
            }
            display.display();
        }
    }

    // Delay between readings
    delay(MEASUREMENT_TIME);
}
