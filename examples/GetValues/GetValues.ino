/** =========================================================================
 * @example{lineno} GetValues.ino
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 * @copyright Stroud Water Research Center
 * @license This example is published under the BSD-3 license.
 *
 * @brief This prints basic meta-data about a sensor to the first serial port and then
 * begins taking measurements from the sensor.
 *
 * The sensor model and address can easily be modified to use this sketch with any
 * Yosemitech modbus sensor.
 *
 * @m_examplenavigation{example_get_values,}
 * @m_footernavigation
 * ======================================================================= */

// ==========================================================================
//  Include the libraries required for any data logger
// ==========================================================================
#include <Arduino.h>
#include <YosemitechModbus.h>


// ==========================================================================
//  Sensor Settings
// ==========================================================================
// Define the sensor type
yosemitechModel model = Y513;  // The sensor model number

// Define the sensor's modbus address, or SlaveID
// NOTE: YosemiTech Windows software presents SlaveID as an integer (decimal),
// whereas EnviroDIY and most other modbus systems present it in hexadecimal form.
// Use an online "HEX to DEC Converter".
byte modbusAddress = 0x01;  // Yosemitech ships sensors with default ID 0x01

// The Modbus baud rate the sensor uses
int32_t modbusBaud =
    9600;  // 9600 is the default baud rate for most sensors


// Time in milliseconds after powering up for the slave device to respond
#define WARM_UP_TIME 1500
           // DO responds within 275-300ms;
           // Turbidity and pH within 500ms
           // Conductivity doesn't respond until 1.15-1.2s

// Time in milliseconds for brush cycle to complete.
// No readings should be taken during this time.
#define BRUSH_TIME 10000
           // On wipered (self-cleaning) models, the brush immediately activates after
           // getting power and takes approximately 10-15 seconds to finish.
           // Turbidity takes 10-11 s
           // Ammonium takes 15 s

// Time in milliseconds for readings to stabilize.
#define STABILIZATION_TIME 2000
// The modbus manuals recommend the following stabilization times between starting
// measurements and requesting values (times include brushing time):
//  2 s for chlorophyll with wiper
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
//  Data Logger Options
// ==========================================================================
const int32_t serialBaud = 115200;  // Baud rate for serial monitor

// Define pin number variables
const int sensorPwrPin  = 10;  // The pin sending power to the sensor
const int adapterPwrPin = 22;  // The pin sending power to the RS485 adapter
const int DEREPin       = -1;  // The pin controlling Receive Enable and Driver Enable
                               // on the RS485 adapter, if applicable (else, -1)
                               // Setting HIGH enables the driver (arduino) to send text
                               // Setting LOW enables the receiver (sensor) to send text

// Turn on debugging outputs (i.e. raw Modbus requests & responses)
// by uncommenting next line (i.e. `#define YM_DEBUG`)
// #define DEBUG


// ==========================================================================
// Create and Assign a Serial Port for Modbus
// ==========================================================================
// Hardware serial ports are preferred when available.
// AltSoftSerial is the most stable alternative for modbus.
//   Select over alternatives with the define below.
// #define BUILD_ALTSOFTSERIAL  // Comment-out if you prefer alternatives

#if defined(BUILD_ALTSOFTSERIAL) && defined(__AVR__)
#include <AltSoftSerial.h>
AltSoftSerial modbusSerial;

#elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_FEATHER328P)
// The Uno only has 1 hardware serial port, which is dedicated to communication with the
// computer. If using an Uno, you will be restricted to using AltSofSerial or
// SoftwareSerial
#include <SoftwareSerial.h>
const int      SSRxPin = 10;  // Receive pin for software serial (Rx on RS485 adapter)
const int      SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)
#pragma message("Using Software Serial for the Uno on pins 10 and 11")
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

#elif defined(ESP8266)
#include <SoftwareSerial.h>
#pragma message("Using Software Serial for the ESP8266")
SoftwareSerial modbusSerial;

#elif defined(NRF52832_FEATHER) || defined(ARDUINO_NRF52840_FEATHER)
#pragma message("Using TinyUSB for the NRF52")
#include <Adafruit_TinyUSB.h>
HardwareSerial& modbusSerial = Serial1;

#elif !defined(NO_GLOBAL_SERIAL1) && !defined(STM32_CORE_VERSION)
// This is just a assigning another name to the same port, for convenience
// Unless it is unavailable, always prefer hardware serial.
#pragma message("Using HardwareSerial / Serial1")
HardwareSerial& modbusSerial = Serial1;

#else
// This is just a assigning another name to the same port, for convenience
// Unless it is unavailable, always prefer hardware serial.
#pragma message("Using HardwareSerial / Serial")
HardwareSerial& modbusSerial = Serial;
#endif

// Construct the Yosemitech modbus instance
yosemitech sensor;

// Initialize success flag for set commands
bool success;


// ==========================================================================
// Working Functions
// ==========================================================================
// A function for pretty-printing the Modbuss Address in Hexadecimal notation,
// from ModularSensors `sensorLocation()`
String prettyprintAddressHex(byte _modbusAddress) {
    String addressHex = F("0x");
    if (_modbusAddress < 0x10) { addressHex += "0"; }
    addressHex += String(_modbusAddress, HEX);
    return addressHex;
}


// ==========================================================================
//  Arduino Setup Function
// ==========================================================================
void setup() {
    // Set various pins as needed
    if (DEREPin >= 0) { pinMode(DEREPin, OUTPUT); }
    if (sensorPwrPin >= 0) {
        pinMode(sensorPwrPin, OUTPUT);
        digitalWrite(sensorPwrPin, HIGH);
    }
    if (adapterPwrPin >= 0) {
        pinMode(adapterPwrPin, OUTPUT);
        digitalWrite(adapterPwrPin, HIGH);
    }

    // Turn on the "main" serial port for debugging via USB Serial Monitor
    Serial.begin(serialBaud);

    // Turn on your modbus serial port
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_FEATHER328P) || \
    defined(ARDUINO_SAM_DUE) || not defined(SERIAL_8E1)
    modbusSerial.begin(modbusBaud);
#elif defined(ESP8266)
    const int SSRxPin = 13;  // Receive pin for software serial (Rx on RS485 adapter)
    const int SSTxPin = 14;  // Send pin for software serial (Tx on RS485 adapter)
    modbusSerial.begin(modbusBaud, SWSERIAL_8N1, SSRxPin, SSTxPin, false);
#else
    modbusSerial.begin(modbusBaud);
#endif

    // Start up the Yosemitech sensor
    sensor.begin(model, modbusAddress, &modbusSerial, DEREPin);

// Turn on debugging
#ifdef YM_DEBUG
    sensor.setDebugStream(&Serial);
#endif

    // Start up note
    Serial.print(F("\nYosemitech "));
    Serial.print(sensor.getModel());
    Serial.print(F(" sensor for "));
    Serial.println(sensor.getParameter());

    // Allow the sensor and converter to warm up
    Serial.println(F("\nWaiting for sensor and adapter to be ready."));
    Serial.print(F("    Warm up time (ms): "));
    Serial.println(WARM_UP_TIME);
    delay(WARM_UP_TIME);

    // Confirm Modbus Address
    Serial.println(F("\nExpected modbus address:"));
    Serial.print(F("    Decimal: "));
    Serial.print(modbusAddress, DEC);
    Serial.print(F(", Hexidecimal: "));
    Serial.println(prettyprintAddressHex(modbusAddress));

    // Get the current Modbus Address
    Serial.println(F("Getting current modbus address..."));
    byte id = sensor.getSlaveID();
    Serial.print(F("    Decimal: "));
    Serial.print(id, DEC);
    Serial.print(F(", Hexidecimal: "));
    Serial.println(prettyprintAddressHex(id));

    if (id != 0xFF && id != modbusAddress) {
        Serial.println(F("\nDiscovered modbus address different than expected!"));
        Serial.print(F("Updating sensor modbus address to: "));
        modbusAddress = id;
        Serial.println(prettyprintAddressHex(modbusAddress));
        // Restart the sensor with the discovered address
        sensor.begin(model, modbusAddress, &modbusSerial, DEREPin);
        delay(1500);
    };

    // Get the sensor serial number
    Serial.println(F("\nGetting sensor serial number."));
    String SN = sensor.getSerialNumber();
    Serial.print(F("    Serial Number: "));
    Serial.println(SN);

    // Get the sensor's hardware and software version
    Serial.println(F("Getting sensor version numbers."));
    float hardwareV, softwareV;
    sensor.getVersion(hardwareV, softwareV);
    Serial.print(F("    Current Hardware Version: "));
    Serial.println(hardwareV);
    Serial.print(F("    Current Software Version: "));
    Serial.println(softwareV);

    // Get the sensor calibration equation / status (pH only)
    switch (model) {
        case Y532:  // pH, calibration status
        {
            Serial.println(F("Getting sensor calibration status."));
            byte status = sensor.pHCalibrationStatus();
            Serial.print(F("    Status: 0x0"));
            Serial.println(status, HEX);
            break;
        }
        case Y4000: {
            Serial.println("For Y4000, use YosemiTech software to get "
                           "calibration parameters.");
            break;
        }
        default:  // Get the sensor's current calibration values
        {
            Serial.println(F("Getting sensor calibration equation."));
            float Kval = 0;
            float Bval = 0;
            sensor.getCalibration(Kval, Bval);
            Serial.print(F("    Current Calibration Equation: final = "));
            Serial.print(Kval);
            Serial.print(F("*raw + "));
            Serial.println(Bval);
        }
            Serial.println();
    }

    // Get/set the sensor brush status (for sensors with brushes).
    // NOTE: Not implemented for Y4000
    if (model == Y511 || model == Y513 || model == Y514 || model == Y551 ||
        model == Y560) {
        // Check the wiper timing
        Serial.println(F("\nGetting sensor cleaning interval."));
        uint16_t interval = sensor.getBrushInterval();
        Serial.print(F("    Sensor auto-cleaning interval: "));
        Serial.print(interval);
        Serial.println(F(" minutes"));

        // Reset the wiper interval to 30 minutes, the default
        Serial.println(F("Resetting cleaning interval to 30 minutes."));
        success = sensor.setBrushInterval(30);
        if (success)
            Serial.println(F("    Reset."));
        else
            Serial.println(F("    Set interval failed!"));
    }

    // Activate the brush, for sensors that have a brush.
    if (model == Y511 || model == Y513 || model == Y514 || model == Y551 ||
        model == Y560 || model == Y4000)
    // Y4000 activates brush when powered on
    {
        // We'll run the brush once in the middle of this
        Serial.println(F("Activating brush."));
        success = sensor.activateBrush();
        if (success)
            Serial.println(F("    Brush activated."));
        else
            Serial.println(F("    Failed to activate brush!"));
    }
    // Additional stabilization time for sensors that have a brush to complete a brush
    // cycle.
    if (model == Y511 || model == Y513 || model == Y514 || model == Y551 ||
        model == Y560 || model == Y4000 || model == Y510) {
        Serial.println(F("Waiting to complete brushing cycle.."));
        Serial.print(F("    Brush time (ms): "));
        Serial.println(BRUSH_TIME);
        for (int i = (BRUSH_TIME + 500) / 1000; i > 0; i--) {  // +500 to round up
            Serial.print(i);
            delay(250);
            Serial.print(F("."));
            delay(250);
            Serial.print(F("."));
            delay(250);
            Serial.print(F("."));
            delay(250);
        }
        Serial.println(F("\n"));
    }

    // Tell the sensor to start taking measurements
    Serial.println(F("Starting sensor measurements"));
    success = sensor.startMeasurement();
    if (success) {
        Serial.println(F("    Measurements started."));
    } else {
        Serial.println(F("    Failed to start measuring!"));
    }

    Serial.println(F("Waiting for sensor to stabilize.."));
    Serial.print(F("    Stabilization time (ms): "));
    Serial.println(STABILIZATION_TIME);
    for (int i = (STABILIZATION_TIME + 500) / 1000; i > 0; i--)  // +500 to round up
    {
        Serial.print(i);
        delay(250);
        Serial.print(F("."));
        delay(250);
        Serial.print(F("."));
        delay(250);
        Serial.print(F("."));
        delay(250);
    }
    Serial.println(F("\n"));

    // Print table headers
    switch (model) {
        case Y4000: {
            Serial.print(F("Time(ms) "));
            Serial.println(sensor.getParameter());
            // "DO,   Turb, Cond,  pH,   Temp, ORP,  Chl,  BGA"
            Serial.print(F("ms       "));
            Serial.println(sensor.getUnits());
            // "mg/L, NTU,  mS/cm, pH,   °C,   mV,   µg/L, µg/L"
            break;
        }
        default: {
            Serial.print(F("Time(ms) "));
            Serial.print(F("Temp(°C)  "));
            Serial.print(sensor.getParameter());
            Serial.print(F("("));
            Serial.print(sensor.getUnits());
            Serial.print(F(")"));
            if (model == Y532 || model == Y504) { Serial.print(F("    Value")); }
            if (model == Y551) { Serial.print(F("    Turbidity (NTU)")); }
            if (model == Y560) { Serial.print(F("    pH")); }
            Serial.println();
        }
    }
}

// ==========================================================================
//  Arduino Loop Function
// ==========================================================================
void loop() {
    // send the command to get the values
    switch (model) {
        case Y4000: {
            float DOmgL, Turbidity, Cond, pH, Temp, ORP, Chlorophyll, BGA = -9999;
            // byte errorCode = 0xFF;  // Error!

            sensor.getValues(DOmgL, Turbidity, Cond, pH, Temp, ORP, Chlorophyll, BGA);

            Serial.print(millis());
            Serial.print(F("    "));
            Serial.print(DOmgL, 4);
            Serial.print(F("  "));
            Serial.print(Turbidity, 4);
            Serial.print(F("  "));
            Serial.print(Cond, 4);
            Serial.print(F("  "));
            Serial.print(pH, 4);
            Serial.print(F("  "));
            Serial.print(Temp, 4);
            Serial.print(F("  "));
            Serial.print(ORP, 4);
            Serial.print(F("  "));
            Serial.print(Chlorophyll, 4);
            Serial.print(F("  "));
            Serial.print(BGA, 4);
            Serial.println();
            break;
        }
        default: {
            float parmValue, tempValue, thirdValue = -9999;
            sensor.getValues(parmValue, tempValue, thirdValue);

            Serial.print(millis());
            Serial.print(F("     "));
            Serial.print(tempValue, 4);
            Serial.print(F("   "));
            Serial.print(parmValue, 4);
            if (model == Y532 || model == Y504 || model == Y551 || model == Y560) {
                Serial.print(F("          "));
                Serial.print(thirdValue, 4);
            }
            Serial.println();
        }
    }

    // Delay between readings
    delay(MEASUREMENT_TIME);
}
