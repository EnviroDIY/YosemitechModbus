int State8 = LOW;
int State9 = LOW;
int incomingByte = 0; // for incoming serial data
unsigned char buffer[13];
unsigned char command[16];
float Temperature, DOvalue;
unsigned char startmeasure[8] = {0x01, 0x03, 0x25, 0x00, 0x00, 0x01, 0x8F, 0x06}; unsigned char getTempandDO[8] = {0x01, 0x03, 0x26, 0x00, 0x00, 0x04, 0x4F, 0x41}; int i = 0;
int inbyte;
String inputString = "";
union SeFrame
{
float Float;
unsigned char Byte[4]; };
SeFrame Sefram;
float Rev_float( unsigned char indata[], int stindex) {
Sefram.Byte[0] = indata[stindex];//Serial.read( ); Sefram.Byte[1] = indata[stindex + 1]; //Serial.read( ); Sefram.Byte[2] = indata[stindex + 2]; //Serial.read( ); Sefram.Byte[3] = indata[stindex + 3]; //Serial.read( ); return Sefram.Float;
}
float Rev_float(unsigned char indata[], int stindex); void setup() {
pinMode(8, OUTPUT);
pinMode(9, OUTPUT);
pinMode(22, OUTPUT);
digitalWrite(22, HIGH);
pinMode(12, OUTPUT);
digitalWrite(12, LOW);
Serial.begin(9600);
Serial1.begin(9600);
//digitalWrite(12, HIGH);
delay(8);
Serial1.write(startmeasure, 8);///////////////////////////
//this is the Mayfly's default Xbee port (UART-1)
}
//delay(12); //digitalWrite(12, LOW);
delay(1000);
if (Serial1.available() > 0) {
// read the incoming byte:
incomingByte = Serial1.readBytes(buffer, 13); }
Serial.println("Temp(C)
void loop() {
if (State8 == LOW) {
State8 = HIGH; } else {
State8 = LOW; }
DO(%)");
digitalWrite(8, State8);
State9 = !State8;
digitalWrite(9, State9);
// send data only when you receive data: //digitalWrite(12, HIGH);
//delay(8);
if (Serial.available() > 0) {
incomingByte = Serial.readBytes(command,17); // if ((incomingByte == 8)||(incomingByte == 17))
//{
Serial1.write(command, incomingByte); //Serial.println("K B");
// }
} else
Serial1.write(getTempandDO, 8); //delay(32);
//digitalWrite(12, LOW); delay(1000);
if (Serial1.available() > 0) {
// read the incoming byte:
incomingByte = Serial1.readBytes(buffer, 13); //default to 1 second
// say what you got:
if (incomingByte == 13) {
Temperature = Rev_float(buffer, 3); DOvalue = Rev_float(buffer, 7) * 100; Serial.print(Temperature, 2); Serial.print(" "); Serial.println(DOvalue, 2);
}
//Serial.print(buffer[0], HEX); }
}
