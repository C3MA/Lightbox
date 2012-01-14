import de.c3ma.artnet.*;

void setup() {
  Artnet a = new Artnet("192.168.2.60" /* The IP address of the Artnet device */, 
                      (byte) 0, /* the universe, that should be used */
                      512 /* maximal length of universe that should be transmitted*/);
  /* send the data */
  a.send( new byte[]{(byte) 255, (byte) 255, (byte) 0, (byte) 255 });
  
  /* you could also specify an offset for the data array: */
    a.send(1, new byte[]{(byte) 255, (byte) 0, (byte) 255 });
}
