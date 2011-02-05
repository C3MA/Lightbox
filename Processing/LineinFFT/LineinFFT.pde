/**
 * This sketch demonstrates how to use an FFT (Fast Fourier Transform) to analyze an
 * AudioBuffer and draw the resulting spectrum. <br />
 * It also allows you to turn windowing on and off,
 * but you will see there is not much difference in the spectrum.<br />
 * Press 'w' to turn on windowing, press 'e' to turn it off.
 */

import processing.net.*; 
Client networkClient; 

import ddf.minim.analysis.*;
import ddf.minim.*;
import controlP5.*;
import processing.net.*;
ControlP5 controlP5;

import processing.serial.*;
Serial myPort = null;
Client networkClient = null;

Minim minim;
AudioInput in;
FFT fft;
String windowName;


final int NETWORK_SIZE = 5;

//bangs are used to simulate the lightboxes
Bang[] bang = new Bang[NETWORK_SIZE];
Bang[][] colorInput = new Bang[NETWORK_SIZE][3]; // for each node and each color one bang

boolean slomotion = false;

public void readSettings(String file) { 
  BufferedReader reader;

  if (file == null)
    reader = createReader(getDefaultFile());
  else
    reader = createReader(file);

  String line;
  try {
    while ((line = reader.readLine()) != null) {
      String[] pieces = split(line, TAB);
      // Example line:
      //0       5.0 470.0       5.0 510.0       5.0 550.0
      if (pieces.length < 4)
        continue;
      int position = int(pieces[0]);

      CVector3f r = extractPosition(pieces[1]);
      CVector3f g = extractPosition(pieces[2]);
      CVector3f b = extractPosition(pieces[3]);

      colorInput[position][0].position().x = r.x;
      colorInput[position][0].position().y = r.y;
      colorInput[position][1].position().x = g.x;
      colorInput[position][1].position().y = g.y;
      colorInput[position][2].position().x = b.x;
      colorInput[position][2].position().y = b.y;
    }
  } 
  catch (IOException e) {
    e.printStackTrace();
    line = null;
  }
}

CVector3f extractPosition(String element) {
  String[] t = split(element, " ");
  if (t.length != 2)
    return null;
  return new CVector3f(float(t[0]), float(t[1]), 0f);
}

void setup()
{
  networkClient = new Client(this, "10.23.42.111", 2001);
  
  size(800, 600);
  //    size(512, 200, P3D);
  //  textMode(SCREEN);
  frameRate(25);

  minim = new Minim(this);

  // get a line in from Minim, default bit depth is 16
  in = minim.getLineIn(Minim.STEREO,512);

  // create an FFT object that has a time-domain buffer 
  // the same size as jingle's sample buffer
  // note that this needs to be a power of two 
  // and that it means the size of the spectrum
  // will be 512. see the online tutorial for more info.
  fft = new FFT(in.bufferSize(), in.sampleRate());

  textFont(createFont("Arial", 16));

  strechPeak = round((width * 1.0) / fft.specSize());

  output = new int[NETWORK_SIZE * 4]; // We have three colors in each box available
  slotsize = round((fft.specSize() * 1.0) / NETWORK_SIZE);

  windowName = "None";
  startVisualisationY = height - 150;

  final int diff = strechPeak * slotsize;
  final int r = color(255,0,0);
  final int g = color(0,255,0);
  final int b = color(0,0,255);

  // Offset of the leftest input "LED"
  final int INPUT_OFFSET = 5;

  controlP5 = new ControlP5(this);
  for(int i=0; i < NETWORK_SIZE; i++) {
    bang[i] = controlP5.addBang("bang" + i, 640 + (i*25), 2,20,20);
    bang[i].setId(i);
    bang[i].setCaptionLabel(""+i);
    bang[i].setColorForeground(0);

    colorInput[i][0] = controlP5.addBang("colorRed" + i, INPUT_OFFSET + (i * diff), startVisualisationY + 20, 20, 20);
    colorInput[i][0].setId(i);
    colorInput[i][0].setCaptionLabel(""+i);
    colorInput[i][0].setColorForeground(r);

    colorInput[i][1] = controlP5.addBang("colorGreen" + i, INPUT_OFFSET + (i * diff), startVisualisationY + 60, 20, 20);
    colorInput[i][1].setId(i);
    colorInput[i][1].setCaptionLabel(""+i);
    colorInput[i][1].setColorForeground(g);

    colorInput[i][2] = controlP5.addBang("colorBlue" + i, INPUT_OFFSET + (i * diff), startVisualisationY + 100, 20, 20);
    colorInput[i][2].setId(i);
    colorInput[i][2].setCaptionLabel(""+i);
    colorInput[i][2].setColorForeground(b);
  }

  // open RS232 Port
<<<<<<< HEAD
  //String portName = Serial.list()[0];
  //myPort = new Serial(this, portName, 57600);

=======
  String portName = Serial.list()[0];
  //myPort = new Serial(this, portName, 57600);
  networkClient = new Client(this, "10.23.42.111", 2001);
>>>>>>> 5fddef017bad93d5c240572cbde9da86729bc3c8

  // a Hamming window can be used to shape the sample buffer that is passed to the FFT
  // this can reduce the amount of noise in the spectrum
  fft.window(FFT.HAMMING);
  windowName = "Hamming";

  readSettings(null);

  controlP5.addToggle("Slomotion", false, 700, startVisualisationY + 60, 80, 20);
}


int[] output;
int slotsize;

int strechPeak;
int startVisualisationY;

void draw()
{
  background(0);
  stroke(255);
  // perform a forward FFT on the samples in jingle's left buffer
  // note that if jingle were a MONO file, 
  // this would be the same as using jingle.right or jingle.left
  fft.forward(in.mix);

  //FIXME testing: fft.inverse(in.mix);

  int outi=0;
  int value;

  if (!slomotion) {
    for(int i=0; i < output.length; i++)
      output[i] = 0;
  }

  boolean shrinkValue = false;

  fft.linAverages(output.length);  

  maxValue = 0; // reset the global variable
  for(int i = 0; i < fft.specSize(); i++) // shrink spectrum, and use only 80% of the spectrum
  {
    stroke(255);
    value = ceil(fft.getBand(i) * 4 * (i / 4));
    //    value = (int) (fft.getBand(i) * 5);
    //      value = (int) max( ((fft.getBand(i) - 0.54) * 0.4  ) / 80, startVisualisationY);
    //    value = (int) (fft.getBand(i) * (((i * 2 + 1) >> i) ));



    rect(i* strechPeak, startVisualisationY, strechPeak, -value);

    if (i > 0 && i % slotsize == 0) {
      // draw a horizontal line for each slot
      stroke(color(255,0,0));
      line(outi * (slotsize * strechPeak), startVisualisationY - output[outi], (outi + 1) * (slotsize * strechPeak), startVisualisationY - output[outi]);

      // draw a vertical line to arrange the input bangs
      stroke(color(255,255,255));
      line((outi + 1) * (slotsize * strechPeak), 0, (outi + 1) * (slotsize * strechPeak), height);

      if (slomotion) { // only modify the item once
        if (shrinkValue)
          output[outi] -= 10; // fade slowly to the bottom
      }

      shrinkValue = false;
      outi++;    // use the next slot
      if (outi >= output.length)
        outi = output.length - 1;
    }

    if (slomotion) {
      if (value < output[outi])
        shrinkValue=true;
      else
        output[outi] = value;
    } 
    else {
      output[outi] = max(output[outi],  value);
    }
    maxValue = max(maxValue, value);
  }

  // Display the combined values
  /*  sendPWMCommandToLightBox(0, magic(output[2]), magic(output[0]),   0);
   sendPWMCommandToLightBox(0, magic(output[6]), magic(output[4]),  1);
   sendPWMCommandToLightBox(magic(output[8]), 0, magic(output[10]), 2);
   sendPWMCommandToLightBox(magic(output[12]), magic(output[14]), 0,   3);
   sendPWMCommandToLightBox(0, magic(output[15]), magic(output[16]), 4);*/

  final int diff = strechPeak * slotsize;
  float x, y;
  int r=-1, g=-1, b=-1;
  for(int i=0; i < NETWORK_SIZE; i++) {
    // ---- red ----
    y = colorInput[i][0].position().y;
    if (y < startVisualisationY) {
      x = colorInput[i][0].position().x;
      r = round(x / diff);
    }
    // ---- green ----
    y = colorInput[i][1].position().y;
    if (y < startVisualisationY) {
      x = colorInput[i][1].position().x;
      g = round(x / diff);
    }
    // ---- blue ----
    y = colorInput[i][2].position().y;
    if (y < startVisualisationY) {
      x = colorInput[i][2].position().x;
      b = round(x / diff);
    }
    sendPWMCommandToLightBox(magic(r >= 0 ? output[r] : 0), magic(g >= 0 ? output[g] : 0), magic(b >= 0 ? output[b] : 0), i);
    r=-1; 
    g=-1; 
    b=-1;
  }

  fill(255);
  // keep us informed about the window being used
  text("The window being used is: " + windowName + ".    Drag the colors from each box over the sectrum.", 5, 20);
}


public String getDefaultFile() {
  String output = System.getProperty("user.home");
  output += "/.c3maLb";
  output += "/positions.txt";
  return output;
}

void keyReleased()
{  
  if ( key == 's' )
  {
    slomotion = !slomotion; 
    if (slomotion)
      windowName = "Slomotion activated";
    else
      windowName = "Hamming";
  }
  else if ( key == 'w' ) // Write the actual position into a configuration file
  {
    writeSettings();
  }
}

public void writeSettings() {
  String output = System.getProperty("user.home");
  output += "/.c3maLb";

  // create folder
  new File(output).mkdir();

  println("Homefolder : " + output);

  PrintWriter outputPositions;
  outputPositions = createWriter(getDefaultFile());

  float y, x;
  for(int i=0; i < NETWORK_SIZE; i++) {
    outputPositions.print(""+i);
    for(int j=0; j < 3; j++) { // store each color
      y = colorInput[i][j].position().y;
      x = colorInput[i][j].position().x;
      outputPositions.print("\t" + x + " " + y);
    }
    outputPositions.println();
  }
  outputPositions.flush(); // Writes the remaining data to the file
  outputPositions.close(); // Finishes the file
}

void stop()
{
  // always close Minim audio classes when you finish with them
  in.close();
  minim.stop();

  super.stop();

  // always store the actual setting
  writeSettings();
  if (networkClient != null)
    networkClient.stop();
}

synchronized void sendPWMCommandToLightBox(int r, int g, int b, int id) {
  bang[id].setColorForeground(color(r,g,b));
  String command = "pw";
  command += hex(r,2);
  command += hex(g,2);
  command += hex(b,2);
  command += hex(id,2);
  command += "o";
  sendStringCommandToLightBox(command);
}

synchronized void sendStringCommandToLightBox(String cmd) {
<<<<<<< HEAD
  networkClient.write(cmd);
  //myPort.write(cmd);
=======
  if (myPort != null)
    myPort.write(cmd);
  if (networkClient != null)
    networkClient.write(cmd);
>>>>>>> 5fddef017bad93d5c240572cbde9da86729bc3c8
  println(cmd);
}

int maxValue;

int magic(int number) {
  //  return (int) (255.0 * number / maxValue);
  return number; //TODO No Magic no longer
}

