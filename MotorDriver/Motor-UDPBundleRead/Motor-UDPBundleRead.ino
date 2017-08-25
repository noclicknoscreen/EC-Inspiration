/*---------------------------------------------------------------------------------------------

  Written, maintained by Dudley smith / Pierre-Gilles Levallois
  V 0.1.0

   DHCP is not stable, enough  IP are static

  Le feather reçoit un bundle Osc 0 (fermer le volet) ou 1 (ouvrir le volet)
  S'il reçoit 0 :
   1. Il enclenche le moteur pour fermeture du volet
   2. Il ignore tous les autres messages OSC tant qu'il n'a pas finit la fermeture
   3. Il donne envoi sa position en OSC (le nombre de pas restant à parcourir / Nombre de pas total)

   S'il reçoit 1 :
   1. Il envoie des messages OSC de fermeture des volet à tout ses collègues
   2. Il enclenche le moteur pour ouverture du volet.
   3. Il ignore tous les autres messages OSC tant qu'il n'a pas finit l'ouverture.
   4. Il brodcaste sa position en OSC (le nombre de pas restant à parcourir / Nombre de pas total)

  --------------------------------------------------------------------------------------------- */

/*---------------------------------------------------------------------------------------------
    TODO :
      - variable globale de position à gérer
      - constante définissant l'ip du serveur vidéo (ou alors trouver un moyen de brodcaster la position)
      - Ignorer tous les messages OSC pendant la manoeuvre.

  --------------------------------------------------------------------------------------------- */

// --------------------------------------------------------------------------------------
// Bunch of Ethernet, Wifi, UDP, WebServer and OSC
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

// --------------------------------------------------------------------------------------
// Next, motor driving files
// --------------------------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// --------------------------------------------------------------------------------------
// Generic helping functions
#include <NCNS-ArduinoTools.h>

// --------------------------------------------------------------------------------------
// Global declarations

#define ERROR_LED   0
#define POSTN_LED   4
#define NUMBER_OF_FEATHERS 4

// Structure to declare all the feathers of the install
// This helps a lot keeping this code unique for all devices.
typedef struct {
  String name;
  String mac_address;
  IPAddress ip;
  int totalSteps;
} Feather;

Feather feathers[NUMBER_OF_FEATHERS];
int featherId = 0;  // index of feather in the structure

// --------------------------------------------------------------------------------------
//  PARAMETRAGE DES FEATHERS DE L'INSTALLATION
// --------------------------------------------------------------------------------------
void initFeathers() {
  feathers[0].name = "Feather 1 - Volet de la Cave T";
  feathers[0].mac_address = "60:1:94:19:EC:A8";
  feathers[0].ip = IPAddress(192, 168, 2, 12);
  feathers[0].totalSteps = 50000;

  feathers[1].name = "Feather 2 - Volet de la Cave Z";
  feathers[1].mac_address = "5C:CF:7F:3A:1B:8E";
  feathers[1].ip = IPAddress(192, 168, 2, 13);
  feathers[1].totalSteps = 5000;

  feathers[2].name = "Feather 3 - Volet de la Cave Y";
  feathers[2].mac_address = "5C:CF:7F:3A:39:41";
  feathers[2].ip = IPAddress(192, 168, 2, 14);
  feathers[2].totalSteps = 3000;

  feathers[3].name = "Feather 4 - Volet de la Cave X";
  feathers[3].mac_address = "5C:CF:7F:3A:2D:73";
  feathers[3].ip = IPAddress(192, 168, 2, 15);
  feathers[3].totalSteps = 10000;

}

// --------------------------------------------------------------------------------------
// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60); // Default address, no jumper, no stacking
// Generic motor connected. We need to adjust step number when we will receive the motor
// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
int motorSpeed = 50; // 50 rpm
float currentPosition = 0.0; // We suppose that the store is closed at initial state.

// --------------------------------------------------------------------------------------
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                   // your network password

// --------------------------------------------------------------------------------------
// Web server
ESP8266WebServer server(80);

// --------------------------------------------------------------------------------------
//  Position
unsigned int receivedPosition = 0;              // LOW means led is *on*

/*-----------------------------------------------*
     Handle to /close, /open and /position (or /)
  -----------------------------------------------*/
void handleRoot() {
  Serial.println("Requested '/'");
  server.send(200, "text/json", "{ value : '', message : 'Ici l identification du feather (N°serie, IP, store pilote, etc... et help.' }");
}

void handlePosition() {
  Serial.println("Requested '/position'");
  String message = "{ value : ";
  message += "0.0";
  message += ", message :'' }";
  server.send(200, "text/json", message);
}

void handleOpen() {
  Serial.println("Requested '/open'");
  server.send(200, "text/json", "{ value : '', message : 'Opening the store...' }");
}

void handleClose() {
  Serial.println("Requested '/close'");
  server.send(200, "text/json", "{ value : '', message : 'Closing the store...' }");
}

void handlePause() {
  Serial.println("Requested '/pause'");
  server.send(200, "text/json", "{ value : '', message : 'The store is paused' }");
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

/*
   Return the index in structure of all feather, taking mac adress in account
*/
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

/*-----------------------------------------------*/

void setup() {
  // Serial
  Serial.begin(115200);
  Serial.println("");

  // Feathers definitions
  initFeathers();

  // Pins definitions
  pinMode(ERROR_LED, OUTPUT);
  pinMode(POSTN_LED, OUTPUT);

  // Who Am I ?
  featherId = guessFeather();
  Serial.println("----------------------------------------");
  Serial.print("feather Id : ");
  Serial.println(featherId);
  Serial.print("Name : ");
  Serial.println(feathers[featherId].name);
  Serial.print("Mac Address : ");
  Serial.println(feathers[featherId].mac_address);
  Serial.print( "IP : ");
  Serial.println(feathers[featherId].ip);
  Serial.print("Total Steps Number : ");
  Serial.println(feathers[featherId].totalSteps);
  Serial.println("----------------------------------------");

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

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
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

  // Stepper ignition
  Serial.println("init Stepper with 1.6kHz frequency...");
  AFMS.begin(1600);  // create with the default frequency 1.6KHz
  Serial.print("Set speed to ");
  Serial.print(motorSpeed);
  Serial.println(" rpm");
  myMotor->setSpeed(motorSpeed); // We probably need to adjust speed later in the code to keep the same time between each store.
  Serial.println("Stepper initialized !");
  Serial.println("");

  // Close the store at startup.
  //

  // Web server preparation
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/position", handlePosition);
  server.on("/open", handleOpen);
  server.on("/close", handleClose);
  server.on("/pause", handlePause);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("");

}

/*
   Returning the number of steps for the window opening/closing
   This is a big hack to keep the code generic for the 4 feathers
*/
int getTotalStepsNumber() {

  return 10000;
}

void loop() {

  // Control the connection (led #0)
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    errorBlink(ERROR_LED, 100);

  } else {
    // Handle webserver
    server.handleClient();

    ledBlink(ERROR_LED, 1000);

    // Then wait for OSC
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
}

/*
   Function callback, called at every OSC bundle received
*/
void positionChange(OSCMessage &msg) {
  // Possibly issue onto Millumin, so constrain the values
  float nextPosition = constrain(msg.getFloat(0), 0.0, 1.0);

  receivedPosition = 255 * nextPosition;
  int stepsNumber = int(getTotalStepsNumber() * 0.1);

  analogWrite(POSTN_LED, receivedPosition);

  Serial.print("/position: ");
  Serial.print(nextPosition);
  Serial.print(" receivedPosition : ");
  Serial.println(receivedPosition);
  Serial.print(", Running to the motor for ");
  Serial.print(stepsNumber);
  Serial.println(" steps");

  // how do we know the direction ?
  // 0 = close = downStep
  // 1 = open = upStep
  // We have to store precedent position
  if (nextPosition > 0 && nextPosition < 1) {
    if (currentPosition < nextPosition ) { // Open

    } else { // Close

    }
    currentPosition = nextPosition;
  }

}
