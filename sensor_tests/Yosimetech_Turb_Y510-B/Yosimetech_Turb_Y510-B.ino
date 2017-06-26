/************
Modified by Anthony & Beth
From sketch from YosemiTech for Y511 Turbidity with wiper
************/
#include <Arduino.h>

// Anthony note: Declare variables
int State8 = LOW;
int State9 = LOW;
int incomingByte = 0; // for incoming serial data. Anthony Note: where to store the bytes read
// Anthony Note: "unsigned char" datatype is equivalent to "byte". https://oscarliang.com/arduino-difference-byte-uint8-t-unsigned-cha/
unsigned char buffer[13];   // Anthony Note: Allocate some space for the Bytes, as 13-element array of bytes
unsigned char command[16];  // Anthony Note: Allocate some space for the Bytes, as 16-element array of bytes
float Temperature, VarXvalue;
unsigned char getSlaveAddress[8] = {0xFF, 0x03, 0x30, 0x00, 0x00, 0x01, 0x9E, 0xD4};
unsigned char startmeasure[8] = {0x01, 0x03, 0x25, 0x00, 0x00, 0x00, 0x4E, 0xC6};
                            //   Addr, Fxn , Start Addr,   # Regs  ,    CRC
unsigned char getTempandVarX[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41};
                            //     Addr, Fxn , Start Addr,   # Regs  ,    CRC
int i = 0;      // Anthony note: Index into array; where to store the Bytes
int inbyte;     // Anthony note: Where to store the Bytes read
String inputString = "";

union SeFrame {
  float Float;
  unsigned char Byte[4];
};

SeFrame Sefram;
float Rev_float( unsigned char indata[], int stindex)
{
    Sefram.Byte[0] = indata[stindex];//Serial.read( );
    Sefram.Byte[1] = indata[stindex + 1]; //Serial.read( );
    Sefram.Byte[2] = indata[stindex + 2]; //Serial.read( );
    Sefram.Byte[3] = indata[stindex + 3]; //Serial.read( );
    return Sefram.Float;
}

float Rev_float(unsigned char indata[], int stindex);


#include <SoftwareSerial.h>
SoftwareSerial modbusSerial(10, 11);  // Picking these pins for software serial


void setup()
{
    // set initial pin modes
    pinMode(8, OUTPUT);   // Anthony Note: LED2 green
    pinMode(9, OUTPUT);   // Anthony Note: LED1 red

    pinMode(22, OUTPUT);  // Anthony Note: switched power. 5V to sensor, 3.3V to RS485 adaptor
    digitalWrite(22, HIGH);

    // pinMode(12, OUTPUT);  // SRGD Note: Receive enable/Data enable, iff using MAX485
    // digitalWrite(12, LOW);  // Setting LOW enables the receiver (sensor) to send text

    Serial.begin(9600);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
    // Serial1.begin(9600);  // this is the Mayfly's default Xbee port (UART-1)
    modbusSerial.begin(9600);

    digitalWrite(12, HIGH);  // Setting HIGH enables the driver (mayfly) to send text
    delay(8);  // A short delay for the DE/RE switch

    // modbusSerial.write(getSlaveAddress, 8);  // Send the "start measurement" command
    modbusSerial.write(startmeasure, 8);  // Send the "start measurement" command
    modbusSerial.flush();

    // digitalWrite(12, LOW);  // Setting LOW enables the receiver (sensor) to send text
    delay(2000);  // recommended >2 second delay (see p 15 of manual) after Start Meaurement before Get values

    if (modbusSerial.available() > 0)
    {
        // read the incoming byte:
        incomingByte = modbusSerial.readBytes(buffer, 13);
        Serial.println(incomingByte);
    }

    Serial.println("Temp(C)   TUR(NTU)");
}


void loop()
{
    // Anthony Note: Switch the LED state
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


    // send the command to get the temperature
    // digitalWrite(12, HIGH);  // Setting HIGH enables the driver (mayfly) to send text
    // delay(8);  // A short delay for the DE/RE switch
    modbusSerial.write(getTempandVarX, 8);
    modbusSerial.flush();


    // digitalWrite(12, LOW);  // Setting LOW enables the receiver (sensor) to send text

    delay(1000);  // Delay 1 second for response

    if (modbusSerial.available() > 0)
    {
        // read the incoming byte:
        incomingByte = modbusSerial.readBytes(buffer, 13); //default to 1 second
        // say what you got:
        if (incomingByte == 13)
        {
            Temperature = Rev_float(buffer, 3);
            VarXvalue = Rev_float(buffer, 7);
            Serial.print(Temperature, 2);
            Serial.print("   ");
            Serial.println(VarXvalue, 2);
        }
    }
}
