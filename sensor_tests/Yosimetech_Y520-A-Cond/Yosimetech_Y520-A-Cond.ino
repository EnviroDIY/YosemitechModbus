/*****************************************************************************
Modified by Anthony & Beth
From sketch from YosemiTech for
Y520 CT conductivity sensor
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------
// Anthony note: Declare variables
int State8 = LOW;
int State9 = LOW;
int incomingByte = 0; // for incoming serial data. Anthony Note: where to store the bytes read
// Anthony Note: "unsigned char" datatype is equivalent to "byte". https://oscarliang.com/arduino-difference-byte-uint8-t-unsigned-cha/
unsigned char buffer[14];   // Anthony Note: Allocate some space for the bytes, as 15-element array of bytes (15 for Cond, 13 for all other sensors)
unsigned char command[16];  // Anthony Note: Allocate some space for the bytes, as 17-element array of bytes
float Temperature, Conductivity;
//float Temperature, Conductivity, SN; //Beth note: trying to print serial number in header
unsigned char startmeasure[9] = {0x01, 0x10, 0x1C, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x92};

//unsigned char getTempandCond[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41}; //From the Cond sketch, and Cond always = 0 but T works
unsigned char getTempandCond[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x05, 0x8E, 0x81}; //From Cond Modbus manual, ovf "overflow as floats" error occurs
//unsigned char getSN[8] = {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
int i = 0;      // Anthony note: Index into array; where to store the bytes
int inbyte;     // Anthony note: Where to store the bytes read
String inputString = "";

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
  pinMode(8, OUTPUT);   // Anthony Note: LED2 green
  pinMode(9, OUTPUT);   // Anthony Note: LED1 red
  pinMode(22, OUTPUT);  // Anthony Note: switched power. 5V to sensor, 3.3V to RS485 adaptor
  digitalWrite(22, HIGH);

  Serial.begin(9600);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
  Serial1.begin(9600); //this is the Mayfly's default Xbee port (UART-1)

  delay(8);
  Serial1.write(startmeasure, 9); // byte array of length = 9, see https://www.arduino.cc/en/Serial/Write

  //Beth note: user manual says to wait 10 seconds before conductivity, then to use as average
  delay(7000); // 7 sec delay here, plus 3 second delay below
  //Serial.print("Sesnor SN "); Serial.println(SN); //Beth note: trying to print serial number in header
  Serial.println("Temp(C) Cond(mS/cm)");

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

  digitalWrite(8, State8);  // Anthony Note: Turn on LED2 green if State8 is high
  State9 = !State8;         // Anthony Note: Assign State9 to be NOT State8 (the opposite of State8)
  digitalWrite(9, State9);  // Anthony Note: Turn on LED1 red if State9 is high

  // send data only when you receive data:
  // Anthony note: seems to allow for commands from computer serial monitor to interupt normal loop
  if (Serial.available() > 0)
  {
    Serial.println("Recieving command from computer serial");
    //incomingByte = Serial.readBytes(command, 17); // see https://www.arduino.cc/en/Serial/ReadBytes
    //Serial1.write(command, incomingByte);
  }
  else
    Serial1.write(getTempandCond, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write

  delay(3000); //Beth note: user manual says to wait 3 secs between readings

  if (Serial1.available() > 0)
  {
    // read the incoming byte: see https://www.arduino.cc/en/Serial/ReadBytes
    incomingByte = Serial1.readBytes(buffer, 15); // 15 byte response frame for Cond, according to  the manual
    // say what you got:
    if (incomingByte == 15)
    {
      Temperature = Rev_float(buffer, 3);  // Anthony note: read response frame buffer starting at byte 3
      Conductivity = Rev_float(buffer, 7); // Anthony note: read response frame buffer starting at byte 7
      Serial.print(Temperature, 4);
      Serial.print(", ");
      Serial.println(Conductivity, 4);

      // Print response frame buffer as hexidecimal bytes
      for(int i = 0; i <= 14; i++)
      {
        Serial.print(buffer[i], HEX);
        Serial.print(", ");
      }
      Serial.println("done");

    }
  }
}
