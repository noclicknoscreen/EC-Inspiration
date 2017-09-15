/*---------------------------------------------------------------------------------------------

  Written, maintained by Dudley smith / Pierre-Gilles Levallois
  Noclick.noscreen_
  V 0.9.0

   DHCP is not stable, enough  IP are static

  Le feather reçoit un bundle Osc 0 (fermer le volet) ou 1 (ouvrir le volet)
  S'il reçoit 0 :
   1. Il enclenche le moteur pour fermeture du volet
   2. Il ignore tous les autres messages OSC tant qu'il n'a pas finit la fermeture
   3. Il envoie sa position en OSC (Ferme)

   S'il reçoit 1 :
   1. Il enclenche le moteur pour ouverture du volet.
   2. Il ignore tous les autres messages OSC tant qu'il n'a pas finit l'ouverture.
   3. Il envoie sa position en OSC (Ouvert)

  --------------------------------------------------------------------------------------------- */

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
//#define DEBUG
//#define SERVO_DEBUG

#define NUMBER_OF_FEATHERS 4

// GPIO For feather Huzzah 4, 5, 2, 16, 0, 15
#define ERROR_LED      0
#define FC_UP          5
#define FC_DN          4
#define SERVO_CTRL_PIN 2

// Structure to declare all the feathers of the install
// This helps a lot keeping this code unique for all devices.
typedef struct {
  String name;
  String mac_address;
  IPAddress ip;
  int speed;
} Feather;


Feather feathers[NUMBER_OF_FEATHERS];
int featherId = 0;  // index of feather in the structure

// --------------------------------------------------------------------------------------
//  PARAMETRAGE DES FEATHERS DE L'INSTALLATION
// --------------------------------------------------------------------------------------
void initFeathers() {
  feathers[0].name = "Feather 1 - Volet de la Cave M";
  feathers[0].mac_address = "60:1:94:19:EC:A8";
  feathers[0].ip = IPAddress(192, 168, 2, 12);
  feathers[0].speed = 1; // 1 = 100% of speed

  feathers[1].name = "Feather 2 - Volet de la Cave S";
  feathers[1].mac_address = "5C:CF:7F:3A:1B:8E";
  feathers[1].ip = IPAddress(192, 168, 2, 13);
  feathers[1].speed = 1; // 1 = 100% of speed

  feathers[2].name = "Feather 3 - Volet de la Cave L";
  feathers[2].mac_address = "5C:CF:7F:3A:39:41";
  feathers[2].ip = IPAddress(192, 168, 2, 14);
  feathers[2].speed = 1; // 1 = 100% of speed

  feathers[3].name = "Feather 4 - Volet de la Cave XS";
  feathers[3].mac_address = "5C:CF:7F:3A:2D:73";
  feathers[3].ip = IPAddress(192, 168, 2, 15);
  feathers[3].speed = 1; // 1 = 100% of speed
}

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
//   Return the index in structure of all feather, taking mac adress in account
// --------------------------------------------------------------------------------------
int guessFeather() {
  byte _mac[6];
  WiFi.macAddress(_mac);
  String strMac;
  strMac += String(_mac[0], HEX);
  strMac += ":";
  strMac += String(_mac[1], HEX);
  strMac += ":";
  strMac += String(_mac[2], HEX);
  strMac += ":";
  strMac += String(_mac[3], HEX);
  strMac += ":";
  strMac += String(_mac[4], HEX);
  strMac += ":";
  strMac += String(_mac[5], HEX);
  strMac.toUpperCase();

  for (int i = 0; i < NUMBER_OF_FEATHERS;  i++) {
    if (feathers[i].mac_address == strMac) {
      return i;
    }
  }
  return -1;
}

// --------------------------------------------------------------------------------------
//   Displaying Feather Technical infos
// --------------------------------------------------------------------------------------
String featherInfo() {
  String str = "\n----------------------------------------";
  str += "\nfeather Id : ";
  str += featherId;
  str += "\nName : ";
  str += feathers[featherId].name;
  str += "\nMac Address : ";
  str += feathers[featherId].mac_address;
  str += "\nIP : ";
  str += humanReadableIp(feathers[featherId].ip);
  str += "\nSpeed : ";
  str += feathers[featherId].speed*100;
  str += " %";
  str += "\n----------------------------------------\n";
  return str;
}

// --------------------------------------------------------------------------------------
// Setup
// --------------------------------------------------------------------------------------
void setup()
{

  Serial.begin(115200);
  Serial.println("");

  // Feathers definitions
  initFeathers();
  // Who Am I ?
  featherId = guessFeather();

  // Wifi connection
  // IPs are static, DHCP does not look easy with arduino
  // My static IP
  IPAddress ip = feathers[featherId].ip;
  IPAddress gateway(192, 168, 2, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Connect to WiFi network
  Serial.print("Connecting to SSID [");
  Serial.print(ssid);
  Serial.print("] pass [");
  Serial.print(pass);
  Serial.println("]");

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, pass);

  int k = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (k % 80 == 0) {
      Serial.println("");
    }
    k++;
    ledBlink(ERROR_LED, 100);
    delay(100);
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port : ");
  Serial.println(Udp.localPort());
  Serial.println("");

  // attaches the servo on pin 9 to the servo object
  // And set the center value to 90 (half of 0 - 180)
  Serial.println("init servo-motor with 0° for center value...");
  myServo.setup(SERVO_CTRL_PIN, 0);

  // Set the end-of-course to Simple input
  // Those are 3 contacts
  pinMode(FC_UP, INPUT_PULLUP);
  pinMode(FC_DN, INPUT_PULLUP);

  upState = 0;
  dnState = 0;


  Serial.print(featherInfo());
}

// --------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------
void loop()
{  
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
    myServo.continousRotate(-1);

  } else if (dnState == HIGH) {
    // Turn counterclockwise
    myServo.continousRotate(1);

  } else {
    // Stop
    myServo.maintainCenter();
  }
}

// --------------------------------------------------------------------------------------
//   Stopping the servo by setting all commands to zero
// --------------------------------------------------------------------------------------
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

// --------------------------------------------------------------------------------------
//   Starting the servo by setting all commands to UP
// --------------------------------------------------------------------------------------
void cmd_up() {
  upState = 1;
  dnState = 0;
  startTime = millis();
}

// --------------------------------------------------------------------------------------
//   Starting the servo by setting all commands to DOWN
// --------------------------------------------------------------------------------------
void cmd_dn() {
  upState = 0;
  dnState = 1;
  startTime = millis();
}

// --------------------------------------------------------------------------------------
//   Function callback, called at every OSC bundle received
// --------------------------------------------------------------------------------------
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


