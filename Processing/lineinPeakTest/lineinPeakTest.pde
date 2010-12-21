
//http://code.compartmental.net/tools/minim/manual-beatdetect/

import ddf.minim.*;
import ddf.minim.analysis.*;
//needed to use serial to communicate to the lightbox
import processing.serial.*;
Serial myPort;

Minim minim;
AudioInput in;
BeatDetect beat;


void setup()
{
  size(512, 200, P2D);

  minim = new Minim(this);
  minim.debugOn();

  // get a line in from Minim, default bit depth is 16
  in = minim.getLineIn(Minim.STEREO, 512);


  beat = new BeatDetect(in.bufferSize(), in.sampleRate());
  beat.setSensitivity(150);


  String portName = Serial.list()[0];
  myPort = new Serial(this, portName, 57600);
}

void draw()
{

  beat.detect(in.mix);
  background(0);

  if ( beat.isKick() )
    sendPWMCommandToLightBox(255, 0, 0, 1);
  else
    sendPWMCommandToLightBox(0, 0, 0, 1);

  if ( beat.isSnare() )
    sendPWMCommandToLightBox(0, 255, 0, 1);
  else
    sendPWMCommandToLightBox(0, 0, 0, 1);

  if ( beat.isHat() )
    sendPWMCommandToLightBox(0, 0, 255, 1);
  else
    sendPWMCommandToLightBox(0, 0, 0, 1);    

  stroke(255);

  // draw the waveforms
  for(int i = 0; i < in.bufferSize() - 1; i++)
  {
    line(i, 50 + in.left.get(i)*50, i+1, 50 + in.left.get(i+1)*50);
    line(i, 150 + in.right.get(i)*50, i+1, 150 + in.right.get(i+1)*50);
  }
}

synchronized void sendPWMCommandToLightBox(int r, int g, int b, int id) {
  String command = "pw";
  command += hex(r,2);
  command += hex(g,2);
  command += hex(b,2);
  command += hex(id,2);
  command += "o";
  sendStringCommandToLightBox(command);
}



synchronized void sendStringCommandToLightBox(String cmd) {
  myPort.write(cmd);
  println(cmd);
}

void stop()
{
  // always close Minim audio classes when you are done with them
  in.close();
  minim.stop();

  super.stop();
}

