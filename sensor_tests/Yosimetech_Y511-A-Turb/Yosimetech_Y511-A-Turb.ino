/*****************************************************************************
Modified by Anthony & Beth
From sketch from YosemiTech for
Y511 Turbidity with wiper
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
unsigned char buffer[13];   // Anthony Note: Allocate some space for the Bytes, as 13-element array of bytes
unsigned char command[16];  // Anthony Note: Allocate some space for the Bytes, as 16-element array of bytes
float Temperature, VarXvalue, SN;

unsigned char startmeasure[8] = {0x01, 0x03, 0x25, 0x00, 0x00, 0x01, 0x8F, 0x06};  // from Turb code
// unsigned char startmeasure[8] = {0x01, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4E, 0xC6};  // from Turb manual
                                 // Addr, Fxn , Start Addr, # Register,    CRC
                                 //   01, Read, Coil 9472 ,   0 Regs  ,    CRC
unsigned char getTempandVarX[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41};  // from Turbidity manual
<<<<<<< Updated upstream
                                 // Addr, Fxn , Start Addr, # Register,    CRC
                                 //   01, Read, Coil 9728 ,   4 Regs  ,    CRC
unsigned char getSN[8] =          {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
                                // Addr, Fxn , Start Addr, # Register,    CRC
                                //   01, Read, Coil 2304 ,   7 Regs  ,    CRC
||||||| merged common ancestors

unsigned char getSN[8] = {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
=======

unsigned char getSN[8] = {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94}; 
>>>>>>> Stashed changes
unsigned char activateBrush[9] = {0x01, 0x10, 0x31, 0x00, 0x00, 0x00, 0x00, 0x74, 0x94};
//Beth note: slave ID is byte 1 for all commands. This should be a variable in final code. 
//unsigned char setSlaveID[11] = {0x01, 0x10, 0x30, 0x00, 0x00, 0x01, 0x02, 0x14, 0x00, 0x99, 0x53}; //sets slave ID from 0x01 (at byte 1) to 0x14 (at byte 8)

int i = 0;      // Anthony note: Index into array; where to store the Bytes
int inbyte;     // Anthony note: Where to store the Bytes read
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
    // set initial pin modes
  pinMode(8, OUTPUT);   // Anthony Note: LED2 green
  pinMode(9, OUTPUT);   // Anthony Note: LED1 red

  pinMode(22, OUTPUT);  // Anthony Note: switched power. 5V to sensor, 3.3V to RS485 adaptor
  digitalWrite(22, HIGH);

  Serial.begin(9600);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
  Serial1.begin(9600); //this is the Mayfly's default Xbee port (UART-1)

  delay(8);
  Serial1.write(startmeasure, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write
  delay(8);
  Serial1.write(getSN, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write

  //Modbus manuals recommend the following warm-up times: 2 s for Chl, 20 s for Turb, 10 s for Cond

  delay(2000);  // recommended >2 second delay (see p 15 of manual) after Start Meaurement before Get values

  if (Serial1.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial1.readBytes(buffer, 13);
  }
  //Serial.println(buffer[2]);
  Serial.println("Temp(C)   TUR(NTU)");
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
    Serial1.write(getTempandVarX, 8); // byte array of length = 8, see https://www.arduino.cc/en/Serial/Write

  //Modbus manuals recommend the following remeasure times: 2 s for Chl &Turb, 3 s for Cond
  delay(2000);

  if (Serial1.available() > 0)
  {
    // read the incoming byte: see https://www.arduino.cc/en/Serial/ReadBytes
    incomingByte = Serial1.readBytes(buffer, 13); //default to 1 second
    // say what you got:
    if (incomingByte == 13)
    {
      Temperature = Rev_float(buffer, 3);  // Anthony note: read response frame buffer starting at byte 3
      VarXvalue = Rev_float(buffer, 7);    // Anthony note: read response frame buffer starting at byte 7
      Serial.print(Temperature, 4);
      Serial.print(", ");
      Serial.println(VarXvalue, 4);

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
