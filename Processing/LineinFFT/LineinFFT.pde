/**
  * This sketch demonstrates how to use an FFT (Fast Fourier Transform) to analyze an
  * AudioBuffer and draw the resulting spectrum. <br />
  * It also allows you to turn windowing on and off,
  * but you will see there is not much difference in the spectrum.<br />
  * Press 'w' to turn on windowing, press 'e' to turn it off.
  */
 
import ddf.minim.analysis.*;
import ddf.minim.*;
import controlP5.*;
ControlP5 controlP5;
 
Minim minim;
AudioInput in;
FFT fft;
String windowName;
 
//bangs are used to simulate the lightboxes
Bang[] bang = new Bang[255];

int NETWORK_SIZE = 3;
 
void setup()
{
  size(512, 200);
//    size(512, 200, P3D);
//  textMode(SCREEN);
  frameRate(25);
 
  minim = new Minim(this);
 
  // get a line in from Minim, default bit depth is 16
  in = minim.getLineIn(Minim.STEREO, 512);
 
  // create an FFT object that has a time-domain buffer 
  // the same size as jingle's sample buffer
  // note that this needs to be a power of two 
  // and that it means the size of the spectrum
  // will be 512. see the online tutorial for more info.
  fft = new FFT(in.bufferSize(), in.sampleRate());
 
  textFont(createFont("Arial", 16));
 
  windowName = "None";
  
  controlP5 = new ControlP5(this);
  for(int i=0; i < NETWORK_SIZE; i++){ 
    bang[i] = controlP5.addBang("bang" + i,300 + (i*25), 5,20,20);
    bang[i].setId(i);
    bang[i].setCaptionLabel(""+i);
    bang[i].setColorForeground(0);
  }
  
}
 
void draw()
{
  background(0);
  stroke(255);
  // perform a forward FFT on the samples in jingle's left buffer
  // note that if jingle were a MONO file, 
  // this would be the same as using jingle.right or jingle.left
  fft.forward(in.mix);
  float value;
  
  int box1Red = 0;
  int box1Green = 0;
  int box1Blue = 0;
  int box2Red = 0;
  int box2Green = 0;
  int box2Blue = 0;
  
  for(int i = 0; i < fft.specSize(); i += 2) // only check every second data
  {
    // take the higher one of two neighours
    value = max(fft.getBand(i), fft.getBand(i + 1));
    
    // draw the line for frequency band i, scaling it by 4 so we can see it a bit better
    if ((i >= 0 && i <= 10) || (i >= 60 && i <= 70))
      stroke(color(255, 0, 0));
    else
       stroke(255);
    line(i, height, i, height - value * 4);
    switch(i) {
     case 0:
        box1Blue = (int) value*6;
        break;
      case 6:
        box1Red = (int) value*6;
      case 10:
        box1Green = (int) value*6;
        break;
     case 60: 
        box2Blue = (int) value*6;
     case 66: 
        box2Red = (int) value*6;
     case 70: 
        box2Red = (int) value*6;
        break;
    }
    sendPWMCommandToLightBox(box1Red, box1Green, box1Blue, 0);
    sendPWMCommandToLightBox(box2Red, box2Green, box2Blue, 1);
  }
  fill(255);
  // keep us informed about the window being used
  text("The window being used is: " + windowName, 5, 20);
}
 
void keyReleased()
{
  if ( key == 'w' )
  {
    // a Hamming window can be used to shape the sample buffer that is passed to the FFT
    // this can reduce the amount of noise in the spectrum
    fft.window(FFT.HAMMING);
    windowName = "Hamming";
  }
 
  if ( key == 'e' )
  {
    fft.window(FFT.NONE);
    windowName = "None";
  }
}
 
void stop()
{
  // always close Minim audio classes when you finish with them
  in.close();
  minim.stop();
 
  super.stop();
}

synchronized void sendPWMCommandToLightBox(int r, int g, int b, int id){
  bang[id].setColorForeground(color(r,g,b));
  String command = "pw";
  command += hex(r,2);
  command += hex(g,2);
  command += hex(b,2);
  command += hex(id,2);
  command += "o";
  println(command);
  //TODO
  //sendStringCommandToLightBox(command);
}
