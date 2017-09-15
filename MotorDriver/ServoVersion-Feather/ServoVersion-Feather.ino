// http://www.bajdi.com
// Rotating a continuous servo (tested with a SpringRC SM-S4306R)

// Bunch of Ethernet, Wifi, UDP, WebServer and OSC
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include "NCNS-ArduinoTools.h"
#include "NCNS-Servo.h"

ServoWrapper myServo;

// Uncomment to display debug messages on serial
#define DEBUG
#define SERVO_DEBUG

// GPIO For feather Huzzah 4, 5, 2, 16, 0, 15
#define ERROR_LED      0
#define PIN_UP         2
#define PIN_DN         16
#define FC_UP          5
#define FC_DN          4
#define SERVO_CTRL_PIN 15
#define SERVO_ADJ      A0


// --------------------------------------------------------------------------------------
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                   // your network password

int upState, dnState;
int servoAdjustOld = 0;
float startTime, endTime = 0;


// --------------------------------------------------------------------------------------
//   Reading OSC Bundles, and treat them with an callback
// --------------------------------------------------------------------------------------
void readOSCBundle() {
  OSCBundle bundle;
  int size = Udp.parsePacket();

  if (size > 0) {

    Serial.print(millis());
    Serial.print(" : ");
    Serial.println("OSC Packet as Bundle Received");

    while (size--) {
      // Read and feed the object --
      bundle.fill(Udp.read());
    }

    if (!bundle.hasError()) {
      // Dispatch from Addresses received to callback functions
      bundle.dispatch("/position", positionChange);

    } else {
      // Errors, print them
      OSCErrorCode error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
      // not connected => Message + Blink Lon
      errorBlink(ERROR_LED, 200);

    }
  }
}


// --------------------------------------------------------------------------------------
//   Reading OSC Bundles on the network
// --------------------------------------------------------------------------------------
void sendOSCBundle(IPAddress ip, int port, String path, float value) {
  OSCBundle bundle;

  Serial.print("Sending OSC Bundle to " + humanReadableIp(ip));
  Serial.print(" : " + path + "/");
  Serial.println(value);

  bundle.add("/position").add(value);
  Udp.beginPacket(ip, port);
  bundle.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  bundle.empty(); // empty the bundle to free room for a new one
}

// --------------------------------------------------------------------------------------
// Setup
// --------------------------------------------------------------------------------------
void setup()
{

  Serial.begin(115200);

  // attaches the servo on pin 9 to the servo object
  // And set the center value to 90 (half of 0 - 180)
  myServo.setup(SERVO_CTRL_PIN, -3);

  // Set the pins to PULL_UP (HIGH is default)
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DN, INPUT_PULLUP);
  // Set the end-of-course to Simple input
  // Those are 3 contacts
  pinMode(FC_UP, INPUT_PULLUP);
  pinMode(FC_DN, INPUT_PULLUP);
  // Set the the analog pin to center servo
  pinMode(SERVO_ADJ, INPUT);
  upState = 0;
  dnState = 0;
}

// --------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------
void loop()
{  
  //  upState = digitalRead(PIN_UP);
  //  dnState = digitalRead(PIN_DN);
  //
  // -------------------------------------------------------
  // Reading command on serial
  // -------------------------------------------------------
  if (Serial.available() ) {
    int commande = Serial.read();
#ifdef DEBUG
    Serial.println(char(commande));
#endif
    if (char(commande) == '0') {
      // STOP
      cmd_stop();
    }
    if (char(commande) == '1') {
      // UP
      cmd_up();
    }
    if  (char(commande) == '2') {
      // DN
      cmd_dn();
    }
  }
#ifdef DEBUG
  Serial.print("States [UP, DN] : [");
  Serial.print(upState);
  Serial.print(",");
  Serial.print(dnState);
  Serial.print("]");
#endif
  // -------------------------------------------------------
  // Reading FC sensors
  // -------------------------------------------------------
  int fcUpState = digitalRead(FC_UP);
  int fcDnState = digitalRead(FC_DN);
  if ( (fcUpState == LOW && upState == HIGH)
       || (fcDnState == LOW && dnState == HIGH))
  {
    cmd_stop();
  }

#ifdef DEBUG
  Serial.print(" : ");
  Serial.print("FC States [UP, DN] : [");
  Serial.print(fcUpState);
  Serial.print(",");
  Serial.print(fcDnState);
  Serial.println("]");
#endif

  // -------------------------------------------------------
  // Reading adusting value for servo, if changed
  // -------------------------------------------------------
  /*
  int sensorValue = analogRead(SERVO_ADJ);
  int servoAdjust = map(sensorValue, 0, 1023, -20, 20);
#ifdef DEBUG
  Serial.print("Read : ");
  Serial.print(sensorValue);
  Serial.print(", Adjusting value : ");
  Serial.println(servoAdjust);
#endif

  // Adusting Servo, if value has changed
  if (servoAdjust != servoAdjustOld ) {
    servoAdjustOld = servoAdjust;
    myServo.setup(SERVO_CTRL_PIN, servoAdjust);
  }
  */
  // -------------------------------------------------------
  // Running the SERVO
  // -------------------------------------------------------
  if (upState == HIGH) {
    // Turn clockwise
    // 1 : Full speed clockwise
    // 0 : Would be stop
    // -1 : Full speed counterclockwise
    myServo.continousRotate(1);

  } else if (dnState == HIGH) {
    // Turn counterclockwise
    myServo.continousRotate(-1);

  } else {
    // Stop
    myServo.maintainCenter();
  }
}

/*
   Stopping the servo by setting all commands to zero
*/
void cmd_stop() {
  // STOP
  upState = 0;
  dnState = 0;
  endTime = millis();
  float timeElapsed = (endTime - startTime) / 1000;
  Serial.print("Time elapsed : ");
  Serial.print(String(timeElapsed));
  Serial.println(" seconds.");
}

/*
   Starting the servo by setting all commands to UP
*/
void cmd_up() {
  upState = 1;
  dnState = 0;
  startTime = millis();
}

/*
   Starting the servo by setting all commands to DOWN
*/
void cmd_dn() {
  upState = 0;
  dnState = 1;
  startTime = millis();
}

/*
   Function callback, called at every OSC bundle received
*/
void positionChange(OSCMessage &msg) {
  // Possibly issue onto Millumin, so constrain the values; This should be 0.0 or 1.0
  float nextPosition = constrain(msg.getFloat(0), 0.0, 1.0);

  //receivedPosition = 255 * nextPosition;
  //analogWrite(POSTN_LED, receivedPosition);

#ifdef DEBUG
  Serial.print("/position: ");
  Serial.print(nextPosition);
#endif

  switch (int(nextPosition)) {
    case 0 :
      // close
      cmd_dn();
      break;
    case 1 :
      // open
      cmd_up();
      break;
    default :
      ledBlink(ERROR_LED, 200);
      cmd_stop();
#ifdef DEBUG
      Serial.print("The value was not suitable (");
      Serial.print(nextPosition);
      Serial.println("). This should be 0.0 or 1.0.");
#endif
  }
}


