
import processing.serial.*;
Serial myPort;  // Create object from Serial class

import controlP5.*;
ControlP5 controlP5;

String textValue = "";
Textfield myTextfield;

Slider sliderDelay;
Slider sliderDuration;

DemoThread demoThread;

Bang[] bang = new Bang[255];

int NETWORK_SIZE = 3;

void setup() {
  size(640,480);
  //smooth();
  frameRate(25);
  
  controlP5 = new ControlP5(this);
  
  for(int i=0; i < NETWORK_SIZE; i++){ 
    bang[i] = controlP5.addBang("bang" + i,100 + (i*25),250,20,20);
    bang[i].setId(i);
    bang[i].setCaptionLabel(""+i);
    bang[i].setColorForeground(0);
  }
  
  myTextfield = controlP5.addTextfield("texting",20,10,150,20);
  myTextfield.setCaptionLabel("input ID here and press ENTER so send");
  myTextfield.setFocus(true);
  
  //add the buttons to the screen
  Button tmpB;
  tmpB = controlP5.addButton("buttonStop",0,20,50,80,19);
  tmpB.setCaptionLabel("stop demos");
  tmpB = controlP5.addButton("demoA"     ,0,20,100,80,19);
  tmpB.setCaptionLabel("start DemoA");
  tmpB = controlP5.addButton("demoB"     ,0,20,125,80,19);
  tmpB.setCaptionLabel("start DemoB");
  tmpB = controlP5.addButton("demoC"     ,0,20,150,80,19);
  tmpB.setCaptionLabel("start DemoC");
  tmpB = controlP5.addButton("demoD"     ,0,20,175,80,19);
  tmpB.setCaptionLabel("start DemoD");  
  //addd the knobs for demoB
  sliderDelay = controlP5.addSlider("sliderDelay",1,1000,300, 250,20,100,20);
  sliderDelay.setCaptionLabel("delay");
  sliderDuration = controlP5.addSlider("sliderDuration",1,500,20, 250,60,100,20);
  sliderDuration.setCaptionLabel("duration");
  
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 57600);
}

void draw() {
  background(50);
  
  stroke(255);
  line(5, 200, 635, 200);
}

public void controlEvent(ControlEvent theEvent) {
  println(theEvent.controller().name());
}

public void demoA(int theValue) {
  tellOldThreadToKillItself();
  demoThread = new DemoA();
  demoThread.start();
}

public void demoB(int theValue) {
  tellOldThreadToKillItself();
  demoThread = new DemoB();
  demoThread.start();
}

public void demoC(int theValue) {
  tellOldThreadToKillItself();
  demoThread = new DemoC();
  demoThread.start();
}

public void demoD(int theValue) {
  tellOldThreadToKillItself();
  demoThread = new DemoD();
  demoThread.start();
}

public void buttonStop(int theValue) {
  tellOldThreadToKillItself();
}

void tellOldThreadToKillItself(){
  if(demoThread != null)
  {
     //kill the old thread
    demoThread.endThread = true;
    demoThread = null; 
  }
}

public void texting(String theText) {
  // receiving text from controller texting
  println("a textfield event for controller 'texting': "+theText);
  
  int deviceId = Integer.parseInt(theText); 
  sendInit(deviceId);  
}

synchronized void sendPWMCommandToLightBox(int r, int g, int b, int id){
  bang[id].setColorForeground(color(r,g,b));
  String command = "pw";
  command += hex(r,2);
  command += hex(g,2);
  command += hex(b,2);
  command += hex(id,2);
  command += "o";
  sendStringCommandToLightBox(command);
}

synchronized void sendInit(int id){
  String command = "pi";
  command += hex(0,2); // stuffing bytes
  command += hex(0,2); // stuffing bytes
  command += hex(0,2); // stuffing bytes
  command += hex(id,2); // the id that should be set.
  command += "o";
  sendStringCommandToLightBox(command);
}

synchronized void sendStringCommandToLightBox(String cmd){
  myPort.write(cmd);
  println(cmd);
}



//the base class for the demos, adds the ability 
//to a thread to end itself.
class DemoThread extends Thread{
  public boolean endThread = false;
  
  protected void checkEnd() throws Exception{
    if(endThread == true){
     throw new Exception(); 
    }
  }
}


class DemoA extends DemoThread {
    public void run() {
      try{
        for (int i = 0; i < 10; i = i+1) {
          sendPWMCommandToLightBox(0,0,0,1);
          delay(100);
          sendPWMCommandToLightBox(255,0,0,1);
          delay(20);
          checkEnd();
        }
        checkEnd();
        sendPWMCommandToLightBox(0,0,0,1);
      } catch (Exception e){
        return;
      }
    }
}

class DemoB extends DemoThread {
    public void run() {
      try{
        int flashTime = 0;
        int delayTime = 0;
        
        while(true){
          flashTime = (int)sliderDuration.value();
          delayTime = (int)sliderDelay.value();
          sendPWMCommandToLightBox(0,0,0,1);
          delay(delayTime);
          sendPWMCommandToLightBox(255,0,0,1);
          delay(flashTime);
          checkEnd();
        }
      } catch (Exception e){
        return;
      }
    }
}


class DemoC extends DemoThread {
    public void run() {
      try{
        int posRed = 2;
        int posBlue = 1;
        int posGreen = 0;
        int delayMS = 0;
        
        while(true) {
          delayMS = (int)sliderDelay.value();
          
          for(int i=0; i < NETWORK_SIZE; i++) {          
            checkEnd();
            if (posRed == i)
               sendPWMCommandToLightBox(255,0,0,i);
             else if (posBlue == i)
                sendPWMCommandToLightBox(0,0,255,i);
             else if (posGreen == i)
                sendPWMCommandToLightBox(0,255, 0,i);
             else
                sendPWMCommandToLightBox(0,0,0,i);
          }
          delay(delayMS);
          posRed++;
          posBlue++;
          posGreen++;
          if (posRed >= NETWORK_SIZE)
            posRed = 0;
          if (posBlue >= NETWORK_SIZE)
            posBlue = 0;          
          if (posGreen >= NETWORK_SIZE)
            posGreen = 0;          
          checkEnd(); 
        }
      } catch (Exception e){
        return;
      }
    }
}


class DemoD extends DemoThread {
    public void run() {
      try{
        int delayMS = 0;
        int colorValue = 0;
        
        while(true) {
          delayMS = (int)sliderDelay.value();
          
          colorValue+= 1;
          colorValue = colorValue % 256;
          
                sendPWMCommandToLightBox(0,0,colorValue,0);
                sendPWMCommandToLightBox(0,colorValue, 0,1);
          delay(delayMS);
          
          checkEnd(); 
        }
      } catch (Exception e){
        return;
      }
    }
}

/*
class DemoC extends DemoThread {
    public void run() {
      try{
        checkEnd();
      } catch (Exception e){
        return;
      }
    }
}
*/
