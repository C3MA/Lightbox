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

//needed to use serial to communicate to the lightbox
import processing.serial.*;
Serial myPort;
 
 
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
  
  
  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 57600);
}
 
void draw()
{
  background(0);
  stroke(255);
  // perform a forward FFT on the samples in jingle's left buffer
  // note that if jingle were a MONO file, 
  // this would be the same as using jingle.right or jingle.left
  fft.forward(in.mix);
  
  int[] output = new int[20];
  for(int i=0; i < output.length; i++) {
    output[i] = 0;
  }
  
  int outi=0;
  int slotsize = fft.specSize() / output.length;
  
  for(int i = 0; i < fft.specSize(); i += 1) // shrink spectrum, and use only 80% of the spectrum
  {
    stroke(255);
    line(i, height, i, height - fft.getBand(i) * 4);

    if (i > 0 && i % slotsize == 0) {
      // draw a horizontal line for each slot
      stroke(color(255,0,0));
      line(outi * slotsize, height - output[outi], (outi + 1) * slotsize, height - output[outi]);
      outi++;    // use the next slot
      if (outi >= output.length)
        outi = output.length - 1;
    }
    output[outi] = max(output[outi], (int) fft.getBand(i) * 6);
  }
  
  // Display the combined values  
  sendPWMCommandToLightBox(output[2], output[1], output[0], 0);
  sendPWMCommandToLightBox(output[3], output[4], output[5], 1);
    
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
  sendStringCommandToLightBox(command);
}


synchronized void sendStringCommandToLightBox(String cmd){
  myPort.write(cmd);
  println(cmd);
}
