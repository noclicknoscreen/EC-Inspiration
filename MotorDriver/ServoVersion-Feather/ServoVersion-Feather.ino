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

   Pilotage OSC :
   --------------
   -> /position : le feather lit un entier 0 ou 1
   qui donne l'ordre d'ouverture ou de fermeture du volet

   -> /adjust :  le feather lit un entier entre -20 et 20 q
   ui permet d'ajuster le centrage du servo-moteur.

   Pilotage Serial :
   -----------------
   -> envoyer 1 pour monter
   -> envoyer 2 pour descendre
   -> envoyer 0 pour stopper

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
#define DEBUG
#//define SERVO_DEBUG

#define NUMBER_OF_FEATHERS 4

// GPIO For feather Huzzah 4, 5, 2, 16, 0, 15
#define FC_DN          4
#define FC_UP          5
#define SERVO_CTRL_PIN 2
#define COMMAND_PIN    16
#define ERROR_LED      0

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

// ----------------------------------------------------------
// Do blink a led without delay
// First you have to setup your pin with this line below
// pinMode(LED_BUILTIN, OUTPUT);
// ----------------------------------------------------------
void ledBlink(unsigned int _led, unsigned int _delayMs) {
  float alternativeSignal = (millis() % _delayMs) / (float)_delayMs;
  if (alternativeSignal > 0.5) {
    digitalWrite(_led, HIGH);
  } else {
    digitalWrite(_led, LOW);
  }
}

/*************************************************
   Returns a human readable IP from a string
 **************************************************/
String humanReadableIp(IPAddress ip) {
  return String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
}

// --------------------------------------------------------------------------------------
//  PARAMETRAGE DES FEATHERS DE L'INSTALLATION
// --------------------------------------------------------------------------------------
void initFeathers() {
  feathers[0].name = "Feather 1 - Volet de la Cave L";
  feathers[0].mac_address = "60:1:94:19:EC:A8";
  feathers[0].ip = IPAddress(192, 168, 2, 12);
  feathers[0].speed = 1; // 1 = 100% of speed

  feathers[1].name = "Feather 2 - Volet de la Cave S";
  feathers[1].mac_address = "5C:CF:7F:3A:1B:8E";
  feathers[1].ip = IPAddress(192, 168, 2, 13);
  feathers[1].speed = 1; // 1 = 100% of speed

  feathers[2].name = "Feather 3 - Volet de la Cave M";
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
const unsigned int wwwPort = 80;            // www server port

char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                   // your network password

// Web server
ESP8266WebServer server(wwwPort);

// up and down Command
int upState, dnState;
// Keep previous adjust value for servo in memory
int servoAdjustOld = 0;
// Computing elapsed time
float startTime, endTime = 0;

// --------------------------------------------------------------------------------------
//     Handle to /close, /open, /pause and / (root) for infos
// --------------------------------------------------------------------------------------
void handleRoot() {
  Serial.println("Requested '/'");
  String content = "{ value : 0.0, ";
  content += "message : '";
  content += featherInfo();
  content += "\n\n/up to open the store.";
  content += "\n/down to close the store.";
  content += "\n/pause to pause the movement.";
  content +=  "' }";
  server.send(200, "text/json", content);
}

void handleUp() {
  Serial.println("Requested '/open'");
  server.send(200, "text/json", "{ value : '', message : 'Opening the store...' }");
  cmd_up();
}

void handleDown() {
  Serial.println("Requested '/close'");
  server.send(200, "text/json", "{ value : '', message : 'Closing the store...' }");
  cmd_dn();
}

void handlePause() {
  Serial.println("Requested '/pause'");
  server.send(200, "text/json", "{ value : '', message : 'The store is paused...' }");
  cmd_stop();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

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
      Serial.println("Bundle No Error");
      // Dispatch from Addresses received to callback functions
      bundle.dispatch("/position", positionChange);
      bundle.dispatch("/adjust", adjustChange);

    } else {
      // Errors, print them
      OSCErrorCode error = bundle.getError();
      Serial.print("error: ");
      Serial.println(error);
      // not connected => Message + Blink Lon
      ledBlink(ERROR_LED, 200);

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
  str += feathers[featherId].speed * 100;
  str += " %";
  str += "\n----------------------------------------\n";
  return str;
}

// --------------------------------------------------------------------------------------
//   Reading Serial Command
// --------------------------------------------------------------------------------------
void readSerialCommand() {
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

  // Web server preparation
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/up", handleUp);
  server.on("/down", handleDown);
  server.on("/pause", handlePause);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.print("HTTP server started at http://");
  Serial.print(humanReadableIp(feathers[featherId].ip));
  Serial.print(":");
  Serial.print(wwwPort);
  Serial.print("/");
  Serial.println("");

  Serial.print(featherInfo());

// COMMAND PIN
pinMode(COMMAND_PIN, OUTPUT);
  
  
}

// --------------------------------------------------------------------------------------
// Loop
// --------------------------------------------------------------------------------------
void loop()
{
  // -------------------------------------------------------
  // Verifying wifi connection
  // -------------------------------------------------------
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    ledBlink(ERROR_LED, 100);
  } else {
    //
    // Nominal running : Status led gives a 1 sec pulse.
    //
    ledBlink(ERROR_LED, 1000);

    // -------------------------------------------------------
    // Reading OSC Bundles : /position & /adjust
    // -------------------------------------------------------
    readOSCBundle();

    // -------------------------------------------------------
    // Reading command on serial
    // -------------------------------------------------------
    readSerialCommand();

    // -------------------------------------------------------
    // Handle Web server
    // -------------------------------------------------------
    server.handleClient();

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
    // Running the SERVO
    // -------------------------------------------------------
    if (upState == HIGH) {
      // Turn clockwise
      // 1 : Full speed clockwise
      // 0 : Would be stop
      // -1 : Full speed counterclockwise
      myServo.continousRotate(-1);
      ledBlink(COMMAND_PIN, 250);
    } else if (dnState == HIGH) {
      // Turn counterclockwise
      myServo.continousRotate(1);
      ledBlink(COMMAND_PIN, 1000);
    } else {
      // Stop
      myServo.maintainCenter();
      digitalWrite(COMMAND_PIN, LOW);
    }
  } // end else Wifi.status
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
//   Callback Function, called at every OSC bundle received
// --------------------------------------------------------------------------------------
void positionChange(OSCMessage &msg) {
  // Possibly issue onto Millumin, so constrain the values; This should be 0.0 or 1.0
  float nextPosition = constrain(msg.getInt(0), 0.0, 1.0);

  //receivedPosition = 255 * nextPosition;
  //analogWrite(POSTN_LED, receivedPosition);

//#ifdef DEBUG
  Serial.print("/position: ");
  Serial.println(nextPosition);
//#endif

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

// --------------------------------------------------------------------------------------
//   Callback Function, called at every OSC bundle received
// --------------------------------------------------------------------------------------
void adjustChange(OSCMessage &msg) {
  int servoAdjust = constrain(msg.getFloat(0), -50, 50);

  Serial.print("/adjust: ");
  Serial.println(servoAdjust);

  // Adusting Servo, if value has changed
  if (servoAdjust != servoAdjustOld ) {
    servoAdjustOld = servoAdjust;
    myServo.setup(SERVO_CTRL_PIN, servoAdjust);

    Serial.print("Adjust value has changed. Setting it to ");
    Serial.print(servoAdjust);
    Serial.println(".");
  }
}


