// Definition of interrupt names
#include <avr/io.h>
// ISR interrupt service routine
#include <avr/interrupt.h>
#include <EEPROM.h>

/*
accepts only messages with the prefix: p and ended with postfix:o
*/

int CMD_MAX = 128;
char myCmd[128];
int port;

char booted = 0; // bool variable

int deviceid;

int valueRed = 0;
int valueGreen = 0;
int valueBlue = 0;
int demoMode = 1;

#define MAX_BRIGHT  255
#define ID_STORE    1    // The address in the EEPROM where the device ID is stored.

// This is the INT0 Pin of the ATMega8
int progModePin = 2;

// We need to declare the data exchange
// variable to be volatile - the value is
// read from memory.
volatile int programmMode = 0;
volatile unsigned long time;

// Install the interrupt routine.
ISR(INT0_vect) {
  if (millis() < time + 2000)
    return;
    
  time = millis(); 
  // someone has pressed the programmer button
  programmMode = (programmMode) ? 0 : 1;
  Serial.println(programmMode);
  
  if (1 == programmMode) 
    setLedValues(255, 255, 255);
  else if (0 == programmMode)
     setLedValues(0, 0, 0);
}


void setup(){
  deviceid = EEPROM.read(1);
  time = millis(); // initialize time
  deviceid = 1; // Set the device to unknown
  resetPorts();
  Serial.begin(57600);
  
  Serial.println("booted");
  
  int test = analogRead(0);
  randomSeed (test);    // randomize
  Serial.print("Input: ");
  Serial.println(test);
  
  // read from the sense pin
  pinMode(progModePin, INPUT);
  digitalWrite(progModePin, HIGH);
  Serial.println("Processing initialization");
  
  //uncomment to enable buttons
  // Global Enable INT0 interrupt
  GICR |= ( 1 << INT0);
  // Signal change triggers interrupt
  MCUCR |= ( 1 << ISC00);
  MCUCR |= ( 0 << ISC01);
}

void resetPorts()
{
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
   
  analogWrite(9, 10);
  analogWrite(10, 100);
  analogWrite(11, 255);
}

void clearCmdArray(){
  //clear the cmd array
  for (int i = 0; i < CMD_MAX; i++){
    myCmd[i] = '\0';
  }
}

void setLedValues(int r, int g, int b)
{
//   Serial.println("setLedValues");
   analogWrite(9,r);
   analogWrite(10,g);
   analogWrite(11,b);
}


//bsp. pwffbbaa01o
//returns number of read bytes
int readFromSerialIntoCmdArray(){ 
  
  //read from the serial buffer and flush
  int inputSize = Serial.available();
  
  if(inputSize < 11){
    //wait for more input
    return -1;
  }
  
  //there are 11 or more than 11 bytes ready to be read
  
  if(inputSize > 0 && inputSize < CMD_MAX){
    Serial.print("inputSize: ");
    Serial.println(inputSize);
    for (int i = 0; i < inputSize; i++){
      myCmd[i] = Serial.read();
      if(myCmd[i] == 'o')
        return i+1;
    }
  }else if(inputSize >= CMD_MAX){
   Serial.flush();
     Serial.println("too much data, flush");
  }
  return -1;
}


//check if command has the required prefix of 'p'
int checkCmdArrayForPrefix(){
   if (myCmd[0] == 'p'){
      return 1;
   }
  return 0;
}

void sendAckOverSerial(){
  Serial.println("ACK");
}

void sendNackOverSerial(){
  Serial.println("NACK");
}

void sendPingAckOverSerial(){
  Serial.println("PACK");
}

void loop()
{
    
/*  long test = random (1, 80000);
  Serial.print("Test Random: ");
  Serial.println(test);*/
  
  //delay needed to have a chance to get the whole message
  //delay(10);

/*
  if (demoMode == 1 && programmMode == 0) {
    delay(500);
    // some dummy output ... so Pens is happy  
    valueRed += 10;
    valueGreen += 15;
    valueBlue += 5;
    valueRed = valueRed % MAX_BRIGHT;
    valueGreen = valueGreen % MAX_BRIGHT;
    valueBlue = valueBlue % MAX_BRIGHT;
    setLedValues(valueRed, valueGreen, valueBlue);
  }*/
  
  clearCmdArray();
 
  int inputSize = readFromSerialIntoCmdArray();
  if(inputSize != 11)
    return;
    
    
    //debug
    Serial.print("receiced: ");
    Serial.println(myCmd);
    
    int checkCmd = checkCmdArrayForPrefix();
    if(checkCmd == 0){
       //Serial.println("if you dont know what to do type \"ollpehelp\"");
       return; 
    }
   
    // deactivate Demomode
    demoMode = 0;
    
    //check for write command
    if (myCmd[1] == 'w')
    {
      if(programmMode) return;
      
        int red = decodeHex(myCmd[2], myCmd[3]);
        int green = decodeHex(myCmd[4], myCmd[5]);
        int blue = decodeHex(myCmd[6], myCmd[7]);
        int id = decodeHex(myCmd[8], myCmd[9]);
     
        /*Serial.print("red:");
        Serial.println(red);
        Serial.print("green:");
        Serial.println(green);
        Serial.print("blue:");
        Serial.println(blue);
        Serial.print("id:");
        Serial.println(id);*/
        
        if(id == deviceid)
        {
          setLedValues(red, green, blue);
        }
    }
    else if (myCmd[1] == 'i')
    {
      //Only use this functionality, when button is pressed.
      if (programmMode) {
      
       //set the id and save in eeprom
       deviceid = decodeHex(myCmd[8], myCmd[9]);
       Serial.print("deviceid = ");
       Serial.print(deviceid);
       EEPROM.write(1, deviceid);
       
       programmMode = 0;
       Serial.print("NACK");
       // signalise the user, that this box was initialized
       setLedValues(255, 255, 255);
       delay(500);
       setLedValues(0, 0, 0);
       delay(500);
       setLedValues(128, 128, 128);
       delay(500);
       setLedValues(0, 0, 0);
           
      } else { // No programm mode
        Serial.print("NACK");
      }
    }
    else if(myCmd[5] == 'h'  //not working right now
            && myCmd[6] == 'e' 
            && myCmd[7] == 'l' 
            && myCmd[8] == 'p')
    {
      if(programmMode) return;
       sendHelpOverSerial();
    }
    //check for ping command, not working right now
    else if(myCmd[5] == 'd' 
            && myCmd[6] == 'e' 
            && myCmd[7] == 'm' 
            && myCmd[8] == 'o')
    {
      if(programmMode) return;
      demoMode = 1;
      Serial.print("ACK");
    }
    else
    {
      //no write command
       //sendNackOverSerial(); 
    }
    
    //Serial.write(myCmd);   
}

void sendHelpOverSerial()
{
  Serial.println("----help is coming----");
  Serial.println("all commands must be prefixed with \"ollpe\"");
  Serial.println("----commands----");
  Serial.println("wAABBCC01\t set pwm [red][green][blue][id] (Hex)");
  Serial.println("ping\t returns \"PACK\"");
  Serial.println("demo\t starts a demo mode");
  Serial.println("help\t prints this help");
  Serial.println("----help end----");
}


int decodeValue(char* c)
{
  return atoi(c);
}

int decodeHex(char number1, char number2) {
  int value = 0;
  number1 = toupper(number1);
  number2 = toupper(number2);

  value = decodeSingleHex(number1);
  //Serial.print("Number1: ");
  //Serial.println(value);
  value = value << 4;
  //Serial.print("Number1: ");
  //Serial.println(value);
  value += decodeSingleHex(number2);
  //Serial.println("-------------");
  return value; 
}

int decodeSingleHex(char number) {
 if (number >= '0' && number <= '9')
    return number - '0';
  else if (number >= 'A' && number <= 'F') {
    return number - 'A' + 10;
  } 
}
