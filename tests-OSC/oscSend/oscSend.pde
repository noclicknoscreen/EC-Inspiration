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
IFButton voletX, voletS, voletM, voletL;

IFTextField adj1, adj2, adj3, adj4;
IFLabel l, l1, l2, l3, l4, lblLastTag;

IFLookAndFeel closedLook, openedLook;

IFRadioController tagController;
IFRadioButton tagX, tagS, tagM, tagL, tagOff;

public static final int TAG_X_IDX = 3;
public static final int TAG_S_IDX = 1;
public static final int TAG_M_IDX = 0;
public static final int TAG_L_IDX = 2;
public static final int TAG_OFF_IDX = 4;

OscP5 oscP5;
NetAddress adrVoletX, adrVoletS, adrVoletM, adrVoletL;
NetAddress adrMillumin;

int directionX, directionS, directionM, directionL = 1;
int currentColumn, columnX, columnS, columnM, columnL = 0;

void setup() {
  size(640, 480);
  frameRate(25);
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this, 2391);
  adrVoletX = new NetAddress("192.168.2.12", 2390);
  adrVoletS = new NetAddress("192.168.2.13", 2390);
  adrVoletM = new NetAddress("192.168.2.14", 2390);
  adrVoletL = new NetAddress("192.168.2.15", 2390);

  adrMillumin = new NetAddress("127.0.0.1", 5000);

  // Buttons as volets
  c = new GUIController (this);

  voletM = new IFButton ("Volet M", 50, 120, 100, 200);
  voletS = new IFButton ("Volet S", 180, 220, 100, 140);
  voletL = new IFButton ("Volet L", 310, 60, 100, 300);
  voletX = new IFButton ("Volet XS", 440, 200, 100, 100);

  adj1 = new IFTextField("Adjust1", 50, 30, 100);
  adj2 = new IFTextField("Adjust2", 180, 30, 100);
  adj3 = new IFTextField("Adjust3", 310, 30, 100);
  adj4 = new IFTextField("Adjust4", 440, 30, 100);

  tagController = new IFRadioController("Mode Selector");
  tagM = new IFRadioButton ("tag M", 75, 400, tagController);
  tagS = new IFRadioButton ("tag S", 205, 400, tagController);
  tagL = new IFRadioButton ("tag L", 335, 400, tagController);
  tagX = new IFRadioButton ("tag XS", 465, 400, tagController);
  tagOff = new IFRadioButton ("tag Off", 535, 400, tagController);

  l1 = new IFLabel("", 50, 45);
  l2 = new IFLabel("", 180, 45);
  l3 = new IFLabel("", 310, 45);
  l4 = new IFLabel("", 440, 45);
  lblLastTag = new IFLabel("Last tag : ", 75, 450);

  voletX.addActionListener(this);
  voletS.addActionListener(this);
  voletM.addActionListener(this);
  voletL.addActionListener(this);
  /*
  tagX.addActionListener(this);
   tagS.addActionListener(this);
   tagM.addActionListener(this);
   tagL.addActionListener(this);
   */
  tagController.addActionListener(this);

  adj1.addActionListener(this);
  adj2.addActionListener(this);
  adj3.addActionListener(this);
  adj4.addActionListener(this);

  c.add (voletX);
  c.add (voletS);
  c.add (voletM);
  c.add (voletL);

  c.add (adj1);
  c.add (adj2);
  c.add (adj3);
  c.add (adj4);

  c.add (tagX);
  c.add (tagS);
  c.add (tagM);
  c.add (tagL);
  c.add (tagOff);

  c.add (l1);
  c.add (l2);
  c.add (l3);
  c.add (l4);
  c.add (lblLastTag);

  openedLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  openedLook.baseColor = color(0, 0, 0);
  openedLook.textColor = color(255, 255, 255);
  openedLook.borderColor = color(0, 0, 0);
  openedLook.highlightColor = color(170, 170, 170);

  closedLook = new IFLookAndFeel(this, IFLookAndFeel.DEFAULT);
  closedLook.borderColor = color(0, 0, 0);
  closedLook.baseColor = color(255, 255, 255);
  closedLook.highlightColor = color(170, 170, 170);

  voletX.setLookAndFeel(closedLook);
  voletS.setLookAndFeel(closedLook);
  voletM.setLookAndFeel(closedLook);
  voletL.setLookAndFeel(closedLook);

  tagM.setLookAndFeel(closedLook);
  tagS.setLookAndFeel(closedLook);
  tagL.setLookAndFeel(closedLook);
  tagX.setLookAndFeel(closedLook);
  tagOff.setLookAndFeel(closedLook);
}

void draw() {
  background(255);
  /*
  l1.draw();
   l2.draw();
   l3.draw();
   l4.draw();
   lblLastTag.draw();
   */
}

/*
* Sends an osc bundle 0 or 1 by step depending of the direction parameter (-1.0, 1.0)
 */
void sendOSCBundleInt(NetAddress remoteLocation, String address, int value) {
  OscBundle myBundle = new OscBundle();
  println("Sending OSC bundle to " + remoteLocation.toString() );

  OscMessage myMessage = new OscMessage(address);
  myMessage.add(value);

  print(myMessage);
  print(" : ");
  println(value);

  myBundle.add(myMessage);
  myBundle.setTimetag(myBundle.now() + 10000);
  oscP5.send(myBundle, remoteLocation);
}
/*
* Sends an osc bundle 0 or 1 by step depending of the direction parameter (-1.0, 1.0)
 */
void sendOSCBundleFloat(NetAddress remoteLocation, String address, float value) {
  OscBundle myBundle = new OscBundle();
  println("Sending OSC bundle to " + remoteLocation.toString() );

  OscMessage myMessage = new OscMessage(address);
  myMessage.add(value);

  print(myMessage);
  print(" : ");
  println(value);

  myBundle.add(myMessage);
  myBundle.setTimetag(myBundle.now() + 10000);
  oscP5.send(myBundle, remoteLocation);
}

void sendOSCBundleAdjust(NetAddress remoteLocation, IFTextField t) {
  OscBundle myBundle = new OscBundle();
  float val = float(t.getValue());
  println("Sending OSC bundle to " + remoteLocation.toString() );
  println("/adjust/" + val );
  OscMessage myMessage = new OscMessage("/adjust");
  myMessage.add(val);
  myBundle.add(myMessage);
  myBundle.setTimetag(myBundle.now() + 10000);
  oscP5.send(myBundle, remoteLocation);
}

/*
* Changer le libelle du volet
 */
void changeLabelVolet(IFButton v, int direction) {
  if (direction == 0) {
    v.setLabel("Closed");
    v.setLookAndFeel(closedLook);
  } else {
    v.setLabel("Opened");
    v.setLookAndFeel(openedLook);
  }
}


void displayVal(IFLabel l, IFTextField t) {
  l.setLabel(t.getValue());
}

// TAG Section ------------------------------------------------

void newTag(int thisTag) {
  setCurrentColumn(thisTag);

  // Play a millumin column
  sendOSCBundleInt(adrMillumin, "/millumin/action/launchColumn", currentColumn);

  // Changing Vignette media time
  if (currentColumn % 2 == 0) {
    int idxInLayer = (int)random(1, 8);
    // Fade out all
    for (int idxLayer = 1; idxLayer <= 8; idxLayer++) {
      if (idxLayer == idxInLayer) {
        // Fade in one of them
        sendOSCBundleFloat(adrMillumin, "/millumin/index:" + idxLayer+ "/opacity", 1.0);
        sendOSCBundleFloat(adrMillumin, "/millumin/index:" + idxLayer+ "/startMedia", 1.0);
      } else {
        // Fade out the others
        sendOSCBundleFloat(adrMillumin, "/millumin/index:" + idxLayer+ "/opacity", 0.0);
        sendOSCBundleFloat(adrMillumin, "/millumin/index:" + idxLayer+ "/stopMedia", 0.0);
      }
    }
  }
}
  void setCurrentColumn(int thisTag) {

    switch(thisTag) {
    case TAG_X_IDX:
      currentColumn = 3 + 0 + columnX;
      columnX = setColumn(columnX);
      break;

    case TAG_S_IDX:
      currentColumn = 3 + 4 + columnS;
      columnS = setColumn(columnS);
      break;

    case TAG_M_IDX:
      currentColumn = 3 + 8 + columnM;
      columnM = setColumn(columnM);
      break;

    case TAG_L_IDX:
      currentColumn = 3 + 12 + columnL;
      columnL = setColumn(columnL);
      break;

    case TAG_OFF_IDX:
      /*if (currentColumn >= 0 && currentColumn < 8) {
       println("Last Selected = X");
       //currentColumn = 0 + columnX + 1;
       //columnX = setColumn(columnX);
       } else if (currentColumn >= 8 && currentColumn < 16) {
       println("Last Selected = S");
       //currentColumn = 8 + columnS + 1;
       //columnS = setColumn(columnS);
       } else if (currentColumn >= 16 && currentColumn < 24) {
       println("Last Selected = M");
       //currentColumn = 16 + columnM + 1;
       //columnM = setColumn(columnM);
       } else if (currentColumn >= 24 && currentColumn < 32) {
       println("Last Selected = L");
       //currentColumn = 24 + columnL + 1;
       //columnL = setColumn(columnL);
       }*/
      currentColumn = 2;
      break;
    }
  }


  int setColumn(int column) {
    column += 1;
    // Then switch between in and Out
    if (column >= 4) {
      column = 0;
    }
    return column;
  }

  void mousePressed() {
  }

  void actionPerformed (GUIEvent e) {
    // ------------------------------------------------------------------------------
    // Command the doors 
    if (e.getSource() == voletX || e.getSource() == voletS || e.getSource() == voletM || e.getSource() == voletL) {
      if (e.getSource() == voletM) {
        if (directionM == 1) {
          directionM = 0;
        } else {
          directionM = 1;
        }
        sendOSCBundleInt(adrVoletM, "/position", directionM);
        changeLabelVolet(voletM, directionM);
      } else if (e.getSource() == voletS) {
        if (directionS == 1) {
          directionS = 0;
        }  
        sendOSCBundleInt(adrVoletS, "/position", directionS);
        changeLabelVolet(voletS, directionS);
      } else if (e.getSource() == voletX) {
        if (directionX == 1) {
          directionX = 0;
        }  
        sendOSCBundleInt(adrVoletX, "/position", directionX);
        changeLabelVolet(voletX, directionX);
      } else if (e.getSource() == voletL) {
        if (directionL == 1) {
          directionL = 0;
        }  
        sendOSCBundleInt(adrVoletL, "/position", directionL);
        changeLabelVolet(voletL, directionL);
      }
    }

    // For the text fields
    if (e.getSource() == adj1 || e.getSource() == adj2 || e.getSource() == adj3 || e.getSource() == adj4) {
      if (e.getSource() == adj1 && keyPressed == true && keyCode == ENTER) {
        sendOSCBundleAdjust(adrVoletX, adj1);
        displayVal(l1, adj1);
      } else if (e.getSource() == adj2 && keyPressed == true && keyCode == ENTER) {
        sendOSCBundleAdjust(adrVoletS, adj2);
        displayVal(l2, adj2);
      } else if (e.getSource() == adj3 && keyPressed == true && keyCode == ENTER) {
        sendOSCBundleAdjust(adrVoletM, adj3);
        displayVal(l3, adj3);
      } else if (e.getSource() == adj4 && keyPressed == true && keyCode == ENTER) {
        sendOSCBundleAdjust(adrVoletL, adj4);
        displayVal(l4, adj4);
      }
    }

    // ------------------------------------------------------------------------------
    // Command Millumin 
    println("Action performed !!");
    if (e.getSource() == tagX || e.getSource() == tagS || e.getSource() == tagM || e.getSource() == tagL || e.getSource() == tagOff) {
      newTag(tagController.getSelectedIndex());
    }
  }

  /* incoming osc message are forwarded to the oscEvent method. */
  void oscEvent(OscMessage theOscMessage) {

    char lastTag = '?';

    // print the address pattern and the typetag of the received OscMessage
    print("### received an osc message : ");
    print(theOscMessage.toString());
    println();
    print("Addrpattern: ");
    println(theOscMessage.addrPattern());

    if (theOscMessage.addrPattern().contains("/lastTag")) {
      int intLastTag = theOscMessage.get(0).intValue();
      lastTag = (char)intLastTag;
    }

    lblLastTag.setLabel("Last Tag is = " + lastTag);

    print("Last Tag is = ");
    println(lastTag);

    // Launch new tag (Play millumin, etc.)
    switch(lastTag) {
    case 'x':
      newTag(TAG_X_IDX);
      break;
    case 's':
      newTag(TAG_S_IDX);
      break;
    case 'm':
      newTag(TAG_M_IDX);
      break;
    case 'l':
      newTag(TAG_L_IDX);
      break;
    case 'e':
      newTag(TAG_OFF_IDX);
      break;
    }
  }