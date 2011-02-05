// Definition of interrupt names
#include <avr/io.h>
#include <EEPROM.h>

/*
accepts only messages with the prefix: p and ended with postfix:o
*/
int CMD_MAX = 128;
char myCmd[128];
int port;

int deviceid;

int valueRed = 0;
int valueGreen = 0;
int valueBlue = 0;

#define MAX_BRIGHT  255
#define ID_STORE    1    // The address in the EEPROM where the device ID is stored.

// This is the INT0 Pin of the ATMega8
int progModePin = 2;

// We need to declare the data exchange
// variable to be volatile - the value is
// read from memory.
int programmMode = 0;
unsigned long programmBtnTimeDown;
int programmBtnReady = 1;


void setup(){
  resetPorts();
  
  programmBtnTimeDown = millis(); // initialize time
  deviceid = 1; // Set the device to unknown
  Serial.begin(57600);
  
  // read from the sense pin 
  pinMode(progModePin, INPUT);
  digitalWrite(progModePin, HIGH);

  deviceid = EEPROM.read(1);
}

void resetPorts()
{
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  
  analogWrite(9, 100);
  analogWrite(10, 100);
  analogWrite(11, 5);
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
    for (int i = 0; i < inputSize; i++){
      myCmd[i] = Serial.read();
      if(myCmd[i] == 'o')
        return i+1;
    }
  }else if(inputSize >= CMD_MAX){
   Serial.flush();
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

void loop()
{
  
  if(digitalRead(progModePin) == LOW){
    if(programmBtnTimeDown == 0 && programmBtnReady == 1){
      programmBtnTimeDown = millis();
      programmBtnReady = 0;
      //setLedValues(100, 0, 0);
    }
  }else{
    programmBtnTimeDown = 0;
    programmBtnReady = 1;
    //setLedValues(0, 0, 50);
  }
  
  if(programmBtnTimeDown > 0 
      && millis() > programmBtnTimeDown + 500 ){
    // someone has pressed the programmer button for 500ms
    programmMode = (programmMode) ? 0 : 1;
    
    if (1 == programmMode) 
      setLedValues(255, 255, 255);
    else if (0 == programmMode)
       setLedValues(0, 0, 0);
       
    programmBtnTimeDown = 0;
  }
  
  clearCmdArray();
 
  int inputSize = readFromSerialIntoCmdArray();
  if(inputSize != 11)
    return;
    
    int checkCmd = checkCmdArrayForPrefix();
    if(checkCmd == 0){
       //Serial.println("if you dont know what to do type \"ollpehelp\"");
       return; 
    }
   
    
    //check for write command
    if (myCmd[1] == 'w')
    {
      if(programmMode) return;
      
        int red = decodeHex(myCmd[2], myCmd[3]);
        int green = decodeHex(myCmd[4], myCmd[5]);
        int blue = decodeHex(myCmd[6], myCmd[7]);
        int id = decodeHex(myCmd[8], myCmd[9]);
        
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
       byte newdeviceid = decodeHex(myCmd[8], myCmd[9]);
       deviceid = newdeviceid;
       //Serial.print("newdeviceid = ");
       //Serial.println((int)newdeviceid);
       EEPROM.write(1, newdeviceid);
       
       programmMode = 0;
       //Serial.print("NACK");
       // signalise the user, that this box was initialized
       setLedValues(255, 255, 255);
       delay(500);
       setLedValues(0, 0, 0);
       delay(500);
       setLedValues(128, 128, 128);
       delay(500);
       setLedValues(0, 0, 0);
       
       
       //EEPROM.write(1, newdeviceid);
       newdeviceid = EEPROM.read(1);
       //Serial.print("read newdeviceid = ");
       //Serial.println((int)newdeviceid);
           
      } else { 
        // No programming mode
        //do nothing
      }
    }
    else
    {
      //no valid command
      //do nothing
    }
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
