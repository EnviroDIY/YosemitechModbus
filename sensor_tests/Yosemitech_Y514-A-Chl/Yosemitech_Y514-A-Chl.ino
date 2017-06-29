/*****************************************************************************
Modified by Anthony & Beth
From sketch from YosemiTech for
Y514 Turbidity with wiper
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>


// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------
char sensorName[] = "Yosemitech Y514-A Chlorophyl sensor with wiper";
//Modbus manuals recommend the following warm-up times: 2 s for Chl, 20 s for Turb, 10 s for Cond
const int     warmUp = 2000; // in milliseconds
//Modbus manuals recommend the following remeasure times: 2 s for Chl &Turb, 3 s for Cond
const int     remeasure = 2000; // in milliseconds

// Anthony Note: "unsigned char" datatype is equivalent to "byte". https://oscarliang.com/arduino-difference-byte-uint8-t-unsigned-cha/
//unsigned char startMeasure[8] = {0x01, 0x03, 0x25, 0x00, 0x00, 0x01, 0x8F, 0x06};  // from Turb code
unsigned char startMeasure[8] = {0x01, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4E, 0xC6};  // from Chl manual

unsigned char getValues[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x05, 0x8E, 0x81};  // from Chl manual, 15 byte response
const int     getValuesBufferLen = 15; // Length of response frame, in bytes
unsigned char getValuesBuffer[getValuesBufferLen];   // Allocate some space for the Bytes, as n+1 element array of bytes
char var1Name[] = "Temperature(C)"; char var2Name[] = "Chlorophyl(ug/L)";
float var1, var2;

unsigned char getSN[8] = {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
const int     getSNBufferLen = 19; // Length of response frame, in bytes
unsigned char getSNBuffer[getSNBufferLen];   // Allocate some space for the Bytes, as n+1 element array of bytes

unsigned char activateBrush[9] = {0x01, 0x10, 0x31, 0x00, 0x00, 0x00, 0x00, 0x74, 0x94};


int incomingByte = 0; // for incoming serial data. Anthony Note: where to store the bytes read
unsigned char command[16];  // Anthony Note: Allocate some space for the Bytes, as n+1 element array of bytes

int i = 0;      // Anthony note: Index into array; where to store the Bytes
int inbyte;     // Anthony note: Where to store the Bytes read
String inputString = "";

// ---------------------------------------------------------------------------
// Board setup info
// ---------------------------------------------------------------------------
const long SERIAL_BAUD = 9600;  // Serial port baud rate
const int GREEN_LED = 8;  // Pin for the green LED
const int RED_LED = 9;  // Pin for the red LED
int State8 = LOW;
int State9 = LOW;


// ---------------------------------------------------------------------------
// Working Functions
// ---------------------------------------------------------------------------
union SeFrame {
  float Float;
  unsigned char Byte[4];
};

SeFrame Sefram;  // Anthony note: this seems to be creating an object of class "SeFrame", but not sure where that class is defined.
// Declare function to convert 4-byte response to a floating point number
float Rev_float( unsigned char indata[], int stindex)
{
  Sefram.Byte[0] = indata[stindex];//Serial.read( );
  Sefram.Byte[1] = indata[stindex + 1]; //Serial.read( );
  Sefram.Byte[2] = indata[stindex + 2]; //Serial.read( );
  Sefram.Byte[3] = indata[stindex + 3]; //Serial.read( );
  return Sefram.Float;
}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
  // Set up pins for the LED's
  pinMode(GREEN_LED, OUTPUT);   // Anthony Note: LED2 green
  pinMode(RED_LED, OUTPUT);   // Anthony Note: LED1 red

  pinMode(22, OUTPUT);  // Anthony Note: switched power. 5V to sensor, 3.3V to RS485 adaptor
  digitalWrite(22, HIGH);

  Serial.begin(SERIAL_BAUD);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
  Serial1.begin(SERIAL_BAUD); //this is the Mayfly's default Xbee port (UART-1)

  Serial.println(sensorName);

  if (Serial.available() > 0)
  {
    Serial.println("X");
  }
  else
    Serial1.write(getSN, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write

  delay(1000);
  if (Serial1.available() > 0)
  {
    incomingByte = Serial1.readBytes(getSNBuffer, getSNBufferLen);
    Serial.print("SN: ");
    // say what you got:
    if (incomingByte == getValuesBufferLen)
    {
      for(int i = 0; i <= (getSNBufferLen-1); i++)
      {
        Serial.print(getSNBuffer[i], HEX);
        Serial.print(", ");
      }
    }
    Serial.println("end");
  }

  Serial1.write(startMeasure, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write

  //Modbus manuals recommend the following warm-up times: 2 s for Chl, 20 s for Turb, 10 s for Cond
  delay(warmUp);  // recommended >2 second delay (see p 15 of manual) after Start Meaurement before Get values
/*
  if (Serial1.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial1.readBytes(getValuesBuffer, getValuesBufferLen);
  } */
  //Serial.println(getValuesBuffer[2]);
  Serial.print(var1Name); Serial.print(", "); Serial.println(var2Name);
  //Serial.print("Sesnor SN "); Serial.println(SN); //Beth note: trying to print serial number in header
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
  // Anthony Note: Switch State8 high or low, to alternate light colors every loop
  if (State8 == LOW)
  {
    State8 = HIGH;
  }
  else {
    State8 = LOW;
  }
  digitalWrite(GREEN_LED, State8);  // Anthony Note: Turn on LED2 green if State8 is high
  State9 = !State8;         // Anthony Note: Assign State9 to be NOT State8 (the opposite of State8)
  digitalWrite(RED_LED, State9);  // Anthony Note: Turn on LED1 red if State9 is high

  // send data only when you receive data:
  // Anthony note: seems to allow for commands from computer serial monitor to interupt normal loop
  if (Serial.available() > 0)
  {
    Serial.println("Recieving command from computer serial");
    //incomingByte = Serial.readBytes(command, 17); // see https://www.arduino.cc/en/Serial/ReadBytes
    //Serial1.write(command, incomingByte);
  }
  else
    Serial1.write(getValues, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write

  //Modbus manuals recommend the following remeasure times: 2 s for Chl &Turb, 3 s for Cond
  delay(remeasure);

  if (Serial1.available() > 0)
  {
    // read the incoming byte: see https://www.arduino.cc/en/Serial/ReadBytes
    incomingByte = Serial1.readBytes(getValuesBuffer, getValuesBufferLen);
    // say what you got:
    if (incomingByte == getValuesBufferLen)
    {
      var1 = Rev_float(getValuesBuffer, 3);  // Anthony note: read response frame buffer starting at byte 3
      var2 = Rev_float(getValuesBuffer, 7);    // Anthony note: read response frame buffer starting at byte 7
      Serial.print(var1, 4);
      Serial.print(", ");
      Serial.println(var2, 4);

      // Print response frame buffer as hexidecimal bytes
      for(int i = 0; i <= (getValuesBufferLen-1); i++)
      {
        Serial.print(getValuesBuffer[i], HEX);
        Serial.print(", ");
      }
      Serial.println("done");

    }
  }
}
