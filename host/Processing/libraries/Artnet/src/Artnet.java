package de.c3ma.artnet;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

/**
 * created at 13.01.2012 - 22:42:59<br />
 * creator: ollo<br />
 * project: ProcessingArtnet<br />
 * $Id: $<br />
 * @author ollo<br />
 */
public class Artnet implements Runnable {
    
    private DatagramSocket clientSocket;
    private InetAddress IPAddress;
    private int universe_length;
    private byte universe;
    
    private final static int HEADER_LENGTH = 18; 
    private final static int UNIVERSE_SIZE = 512;
    private final static int ARTNET_PORT = 6454;
    

    private final byte[] ARTNET_TEXT = new byte[] { 'A', 'r', 't', '-', 'N', 'e', 't', '\0' };

    public Artnet(final String ip, final byte universe, final int universe_length) {
        try {
            this.clientSocket = new DatagramSocket();
            this.IPAddress = InetAddress.getByName(ip);
            this.universe_length = universe_length;
            this.universe = universe;
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    
    public void send(final byte[] data) {
        send(0, data);
    }
    
    public void send(final int offset, final byte[] data) {
        /*
        char ID[8];             8
        uint16_t opcode;        9, 10
        uint8_t ProtVerH;       11
        uint8_t ProtVerL;       12
        uint8_t Sequence;       13
        uint8_t Physical;       14
        uint8_t SubUni;         15
        uint8_t Net;            16
        uint8_t LengthHi;       17
        uint8_t LengthLo;       18
         */
        
        final int universe_size = Math.min(offset + data.length, Math.min(universe_length, UNIVERSE_SIZE));
        byte[] sendData = new byte[HEADER_LENGTH + universe_size + 1];
        System.arraycopy(ARTNET_TEXT, 0, sendData, 0, ARTNET_TEXT.length);

        sendData[8] = 0x00; /* opcode */
        sendData[9] = 0x50; /* opcode */
        sendData[11] = 1;   /* ProtVerL */
        sendData[14] = universe; /* SubUni */
        sendData[16] = (byte) ((universe_size >> 8) & 0xFF);
        sendData[17] = (byte) (universe_size & 0xFF);
        System.arraycopy(data, 0, sendData, 18, data.length);
        
        DatagramPacket sendPacket = new DatagramPacket(sendData , sendData.length, IPAddress, ARTNET_PORT);
        try {
            clientSocket.send(sendPacket);
            System.out.println("Package was send.");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    @Override
    public void run() {
        // TODO Auto-generated method stub
        
    }

}
