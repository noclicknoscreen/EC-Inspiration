/**
 * oscSend : Programme de test des volets
 * EuroCaves/Inspiration
 * PIerre-Gilles Levallois pour Noclick.noscreen_
 *
 * L'interface comporte 4 volets sur lesquels on clique pour fermer ou ouvrir
 * 
 */

import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress myRemoteLocation;

void setup() {
  size(400,400);
  frameRate(25);
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this,12000);
  
  /* myRemoteLocation is a NetAddress. a NetAddress takes 2 parameters,
   * an ip address and a port number. myRemoteLocation is used as parameter in
   * oscP5.send() when sending osc packets to another computer, device, 
   * application. usage see below. for testing purposes the listening port
   * and the port of the remote location address are the same, hence you will
   * send messages back to this sketch.
   */
  myRemoteLocation = new NetAddress("192.168.2.15",2390);
}


void draw() {
  background(0);  
}

void sendOSCMessage() {
  //OscMessage myMessage = new OscMessage("/position");
  //myMessage.add(value);
  //oscP5.send(myMessage, myRemoteLocation);
  
  /* create an osc bundle */
  OscBundle myBundle = new OscBundle();
  
  for(float i = 0.0; i <= 1.0; i=i+0.05) {
    /* createa new osc message object */
    OscMessage myMessage = new OscMessage("/position");
    myMessage.add(i);
    
    /* add an osc message to the osc bundle */
    myBundle.add(myMessage);
    myBundle.setTimetag(myBundle.now() + 10000);
  }
  /* reset and clear the myMessage object for refill. */
  //myMessage.clear();
  
  ///* refill the osc message object again */
  //myMessage.setAddrPattern("/test2");
  //myMessage.add("defg");
  //myBundle.add(myMessage);
  
  /* send the osc bundle, containing 2 osc messages, to a remote location. */
  oscP5.send(myBundle, myRemoteLocation);
}


void keyPressed() {
  //for(float i = 0.0; i <= 1.0; i=i+0.1) {
    sendOSCMessage();
  //}  
}



/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  print("### received an osc message.");
  print(" addrpattern: "+theOscMessage.addrPattern());
  print(" typetag: "+theOscMessage.typetag());
  println(" timetag: "+theOscMessage.timetag());
}