/*****************************************************************************
Modified by Anthony Aufdenkampe, from GetValues.ino
2018-April
Y4000 MultiParameter Sonde

Depends on EnvrioDIY_SensorModbusMaster, but not YosemitechModbus
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <AltSoftSerial.h>
#include "YosemitechModbus.h"


// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor type
yosemitechModel model = Y4000;  // The sensor model number

// Define the sensor's modbus address
byte modbusAddress = 0x01;  // The sensor's modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor *AND* RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text

// Construct software serial object for Modbus
AltSoftSerial modbusSerial;  // On Mayfly, requires connection D5 & D6

// Construct the modbus instance
modbusMaster modbus;



// ---------------------------------------------------------------------------
// Working Functions
// ---------------------------------------------------------------------------

// Give values to variables;
yosemitechModel _model = model;
byte modbusSlaveID = modbusAddress;
byte _slaveID = modbusSlaveID;

String getSerialNumber(void)
{
    String SN;
    switch (_model)
    {
        case Y4000:
            SN = modbus.StringFromRegister(0x03, 0x1400, 14); break; // for Y4000 Sonde
        default:
            SN = modbus.StringFromRegister(0x03, 0x0900, 14); break; // for all sensors except Y4000
    }

    // Strip out the initial ')' that seems to come with some responses
    if (SN.startsWith(")"))
    {
        SN = SN.substring(1);
    }

    // Return the serial number
    return SN;
}

bool startMeasurement(void)
{
    switch (_model)
    {
        case Y520:
        {
            byte startMeasurementW[9] = {_slaveID, 0x10, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Write, Reg 7168 ,0 Registers, 0byte,    CRC
            int respSize = modbus.sendCommand(startMeasurementW, 9);
            if (respSize == 8 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
            break;
        }
        default:
        {
            byte startMeasurementR[8] = {_slaveID, 0x03, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
                                      // _slaveID, Read,  Reg 9472 ,   0 Regs  ,    CRC
            int respSize = modbus.sendCommand(startMeasurementR, 8);
            if (respSize == 5 && modbus.responseBuffer[0] == _slaveID) return true;
            else return false;
            break;
        }
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

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(9600);  // The modbus serial stream - Baud rate MUST be 9600.

    // Start up the modbus sensor
    modbus.begin(modbusAddress, &modbusSerial, DEREPin);

    Serial.println(getSerialNumber());

    Serial.println(startMeasurement());

    Serial.println('done!');
}


// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{

}
