/************
  Modified by Anthony & Beth
  From sketch from Yosemitech for DO probe
  To work with Y520 CT conductivity
************/

// Anthony note: Declare variables
int State8 = LOW;
int State9 = LOW;
int incomingByte = 0; // for incoming serial data. Anthony Note: where to store the bytes read
// Anthony Note: "unsigned char" datatype is equivalent to "byte". https://oscarliang.com/arduino-difference-byte-uint8-t-unsigned-cha/
unsigned char buffer[13];   // Anthony Note: Allocate some space for the Bytes, as 13-element array of bytes
unsigned char command[16];  // Anthony Note: Allocate some space for the Bytes, as 16-element array of bytes
float Temperature, Conductivity;
byte error;
//float Temperature, Conductivity, SN; //Beth note: trying to print serial number in header
unsigned char startmeasure[9] = {0x01, 0x10, 0x1C, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x92};
//unsigned char getTempandCond[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41}; //This is from the DO sketch, and Cond always = 0 but T works
unsigned char getTempandCond[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x05, 0x8E, 0x81}; //This is from the Conductivity instruction manual, ovf "overflow as floats" error occurs
//unsigned char getTempandCond[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x05, 0x8E, 0x81};
//unsigned char getSN[8] = {0x01, 0x03, 0x09, 0x00, 0x00, 0x07, 0x07, 0x94};
int i = 0;      // Anthony note: Index into array; where to store the Bytes
int inbyte;     // Anthony note: Where to store the Bytes read
int sendstartok = 0;
String inputString = "";

union SeFrame {
  float Float;
  unsigned char Byte[4];
};

SeFrame Sefram;
float Rev_float( unsigned char indata[], int stindex) {
  Sefram.Byte[0] = indata[stindex]; //Serial.read( );
  Sefram.Byte[1] = indata[stindex + 1]; //Serial.read( );
  Sefram.Byte[2] = indata[stindex + 2]; //Serial.read( );
  Sefram.Byte[3] = indata[stindex + 3]; //Serial.read( );
  return Sefram.Float;
}

float Rev_float(unsigned char indata[], int stindex);


void setup()
{
  pinMode(8, OUTPUT);   // Anthony Note: LED2 green
  pinMode(9, OUTPUT);   // Anthony Note: LED1 red
  pinMode(22, OUTPUT);  // Anthony Note: switched power
  digitalWrite(22, HIGH);
  pinMode(12, OUTPUT);  // Anthony Note: ??
  digitalWrite(12, LOW); // Anthony Note: ??
  Serial.begin(9600);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
  Serial1.begin(9600); //this is the Mayfly's default Xbee port (UART-1)
  //digitalWrite(12, HIGH);
  delay(8);
  Serial1.write(startmeasure, 9);///////////////////////////
  //delay(12);
  //digitalWrite(12, LOW);
  delay(10000); //Beth note: user manual says to wait 10 seconds before conductivity, then to use as average


  if (Serial1.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial1.readBytes(buffer, 8);

  }

  Serial.println("Temp(C) Cond(mS/cm)");
  //Serial.print("Sesnor SN "); Serial.println(SN); //Beth note: trying to print serial number in header


}


void loop()
{
  // Anthony Note: Switch State8
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
  //digitalWrite(12, HIGH);
  //delay(8);

  if (Serial.available() > 0)
  {
    incomingByte = Serial.readBytes(command, 17);
    // if ((incomingByte == 8)||(incomingByte == 17))
    //{
    Serial1.write(command, incomingByte);
    //Serial.println("K B");
    // }
  }
  else
  {

    Serial1.write(getTempandCond, 8);
  
  }

  //delay(32);
  //digitalWrite(12, LOW);
  delay(3000); //Beth note: user manual says to wait 3 secs between readings

  if (Serial1.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial1.readBytes(buffer, 15); //default to 1 second
    // say what you got:
    if (incomingByte == 15)
    {
      Temperature = Rev_float(buffer, 3); //Beth: does this mean Rev_float location at byte 3?
      Conductivity = Rev_float(buffer, 7); // Beth note: is this the error in reading conductivity?
      
      Serial.print(Temperature, 6);
      Serial.print(" ");
      Serial.print(Conductivity, 9);
      Serial.print(" ");
      error = Serial.println(buffer[11]);
    }
    //Serial.print(buffer[0], HEX);
  }
}
