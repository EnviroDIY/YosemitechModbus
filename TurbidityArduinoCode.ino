/************
Modified by Anthony & Beth
From sketch from YosemiTech for DO probe
To work with Y520 CT conductivity
************/

// Anthony note: Declare variables
int State8 = LOW;
int State9 = LOW;
int incomingByte = 0; // for incoming serial data. Anthony Note: where to store the bytes read
// Anthony Note: "unsigned char" datatype is equivalent to "byte". https://oscarliang.com/arduino-difference-byte-uint8-t-unsigned-cha/
unsigned char buffer[13];   // Anthony Note: Allocate some space for the Bytes, as 13-element array of bytes
unsigned char command[16];  // Anthony Note: Allocate some space for the Bytes, as 16-element array of bytes
float Temperature, DOvalue;
unsigned char startmeasure[8] = {0x01, 0x10, 0x1C, 0x00, 0x00, 0x00, 0xD8, 0x92};
unsigned char getTempandCond[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41};
int i = 0;      // Anthony note: Index into array; where to store the Bytes
int inbyte;     // Anthony note: Where to store the Bytes read
String inputString = "";

union SeFrame {
  float Float;
  unsigned char Byte[4];
};

SeFrame Sefram;
float Rev_float( unsigned char indata[], int stindex) {
  Sefram.Byte[0] = indata[stindex];//Serial.read( );
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
  digitalWrite(12, LOW);// Anthony Note: ??
  Serial.begin(9600);  // Anthony Note: this is the Mayfly's default USB port (UART-0)
  Serial1.begin(9600); //this is the Mayfly's default Xbee port (UART-1)
  //digitalWrite(12, HIGH);
  delay(8);
  Serial1.write(startmeasure, 8);///////////////////////////
  //delay(12);
  //digitalWrite(12, LOW);
  delay(1000);


  if (Serial1.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial1.readBytes(buffer, 13);
  }

  Serial.println("Temp(C) Cond(mS/cm)");
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
    incomingByte = Serial.readBytes(command,17);
    // if ((incomingByte == 8)||(incomingByte == 17))
    //{
    Serial1.write(command, incomingByte);
    //Serial.println("K B");
    // }
  }
  else
    Serial1.write(getTempandCond, 8);

  //delay(32);
  //digitalWrite(12, LOW);
  delay(1000);

  if (Serial1.available() > 0)
  {
    // read the incoming byte:
    incomingByte = Serial1.readBytes(buffer, 13); //default to 1 second
  // say what you got:
    if (incomingByte == 13)
    {
      Temperature = Rev_float(buffer, 3);
      DOvalue = Rev_float(buffer, 7) * 100;
      Serial.print(Temperature, 2);
      Serial.print(" ");
      Serial.println(DOvalue, 2);
    }
  //Serial.print(buffer[0], HEX);
  }
}
