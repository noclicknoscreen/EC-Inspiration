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
import interfascia.*;

GUIController c;
IFButton volet1, volet2, volet3, volet4;
IFLabel l;

IFLookAndFeel closedLook, openedLook;

OscP5 oscP5;
NetAddress myRemoteLocation1, myRemoteLocation2, myRemoteLocation3, myRemoteLocation4;

float direction = 1;

void setup() {
  size(640,480);
  frameRate(25);
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this,12000);
  myRemoteLocation1 = new NetAddress("192.168.2.12",2390);
  myRemoteLocation2 = new NetAddress("192.168.2.13",2390);
  myRemoteLocation3 = new NetAddress("192.168.2.14",2390);
  myRemoteLocation4 = new NetAddress("192.168.2.15",2390);
  
  // Buttons as volets
  c = new GUIController (this);
  
  volet1 = new IFButton ("Volet 1", 50, 60, 100, 200);
  volet2 = new IFButton ("Volet 2", 180, 60, 100, 100);
  volet3 = new IFButton ("Volet 3", 310, 60, 100, 140);
  volet4 = new IFButton ("Volet 4", 440, 60, 100, 300);

  volet1.addActionListener(this);
  volet2.addActionListener(this);
  volet3.addActionListener(this);
  volet4.addActionListener(this);

  c.add (volet1);
  c.add (volet2);
  c.add (volet3);
  c.add (volet4);

  openedLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  openedLook.baseColor = color(0, 0, 0);
  openedLook.textColor = color(255, 255, 255);
  openedLook.borderColor = color(0, 0, 0);
  openedLook.highlightColor = color(170, 170, 170);

  closedLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  closedLook.borderColor = color(0, 0, 0);
  closedLook.baseColor = color(255, 255, 255);
  closedLook.highlightColor = color(170, 170, 170);

  volet1.setLookAndFeel(closedLook);
  volet2.setLookAndFeel(closedLook);
  volet3.setLookAndFeel(closedLook);
  volet4.setLookAndFeel(closedLook);
}

void draw() {
  background(255);  
}

/*
* Sends an osc bundle from 0 to 1 by step of 0.05
* Or from 1 to 0, depending of the direction parameter (-1.0, 1.0)
*/
void sendOSCBundle(NetAddress remoteLocation) {
  OscBundle myBundle = new OscBundle();
  float increment = 0.1;
  println("-------------------------------------------------------");
  for(float i = 0.0; i < 1.0 + increment; i = i + increment) {
    OscMessage myMessage = new OscMessage("/position");
    if (direction < 0) {
      myMessage.add(1-i);
      println(1-i);
    } else {
      myMessage.add(i);
      println(i);
    }
    myBundle.add(myMessage);
    myBundle.setTimetag(myBundle.now() + 10000);
  }
  oscP5.send(myBundle, remoteLocation);
}

/*
* Sends an osc bundle 0 or 1 by step depending of the direction parameter (-1.0, 1.0)
*/
void sendOSCBundle2(NetAddress remoteLocation) {
  OscBundle myBundle = new OscBundle();
  OscMessage myMessage = new OscMessage("/position");
  if (direction < 0) {
    myMessage.add(0.0);
  } else {
    myMessage.add(1.0);
  }
  myBundle.add(myMessage);
  myBundle.setTimetag(myBundle.now() + 10000);
  oscP5.send(myBundle, remoteLocation);
}

/*
* Sends a unique osc message 1 or 0, depending of the direction parameter (-1.0, 1.0)
*/ 
void sendOSCMessage(NetAddress remoteLocation) {
  if (direction == -1.0 ) {
    direction = 0.0;
  }
  OscMessage myMessage = new OscMessage("/position");
  myMessage.add(direction);
  oscP5.send(myMessage, remoteLocation);
}

/*
* Chnager le libelle du volet
*/
void changeLabel(IFButton v) {
  if (direction == -1.0) {
    v.setLabel("Closed");
    v.setLookAndFeel(closedLook);
  } else {
    v.setLabel("Opened");
    v.setLookAndFeel(openedLook);
  }
}

void mousePressed() {
 
}

void actionPerformed (GUIEvent e) {
  if (e.getSource() == volet1) {
    //sendOSCMessage(myRemoteLocation1);
    //sendOSCBundle(myRemoteLocation1);
    sendOSCBundle2(myRemoteLocation1);
    changeLabel(volet1);

  } else if (e.getSource() == volet2) {
    //sendOSCMessage(myRemoteLocation2);
    //sendOSCBundle(myRemoteLocation2);
    sendOSCBundle2(myRemoteLocation2);
    changeLabel(volet2);

  } else if (e.getSource() == volet3) {
    //sendOSCMessage(myRemoteLocation3);
    //sendOSCBundle(myRemoteLocation3);
    sendOSCBundle2(myRemoteLocation3);
    changeLabel(volet3);

  } else if (e.getSource() == volet4) {
    //sendOSCMessage(myRemoteLocation4);
    //sendOSCBundle(myRemoteLocation4);
    sendOSCBundle2(myRemoteLocation4);
    changeLabel(volet4);
  }
  direction = -direction;
}

/* incoming osc message are forwarded to the oscEvent method. */
void oscEvent(OscMessage theOscMessage) {
  /* print the address pattern and the typetag of the received OscMessage */
  print("### received an osc message.");
  print(" addrpattern: "+theOscMessage.addrPattern());
  print(" typetag: "+theOscMessage.typetag());
  println(" timetag: "+theOscMessage.timetag());
}