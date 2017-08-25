/*---------------------------------------------------------------------------------------------

  Written, maintained by Dudley smith / Pierre-Gilles Levallois
  V 0.1.0

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
#define ERROR_LED   0
#define POSTN_LED   4

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60); // Default address, no jumper, no stacking
// Generic motor connected. We need to adjust step number when we will receive the motor
// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
int motorSpeed = 50; // 50 rpm
float previousPosition = 0.0; // We suppose that the store is closed at initial state.

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                   // your network password


unsigned int receivedPosition = 0;              // LOW means led is *on*

ESP8266WebServer server(80);

/*----------------------------------------------------
   DHCP is not stable, enough
   IP are static
   Down here are the hack of DHCP
  ----------------------------------------------------*/
IPAddress getIp(byte _mac[6]) {

  IPAddress _ip;

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

  Serial.print("MAC Address: ");
  Serial.println(strMac);

  // My Own DHCP Table :((((
  if (strMac == "60:1:94:19:EC:A8") {
    // Feather 1
    _ip = IPAddress(192, 168, 2, 12);
  } else if (strMac == "5C:CF:7F:3A:1B:8E") {
    // Feather 2
    _ip = IPAddress(192, 168, 2, 13);
  } else if (strMac == "5C:CF:7F:3A:39:41") {
    // Feather 3
    _ip = IPAddress(192, 168, 2, 14);
  } else if (strMac == "5C:CF:7F:3A:2D:73") {
    // Feather 4
    _ip = IPAddress(192, 168, 2, 15);
  }

  Serial.print("Obtained IP is : ");
  Serial.println(_ip);

  return _ip;
}

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

/*-----------------------------------------------*/

void setup() {
  // Serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // Pins definitions
  pinMode(ERROR_LED, OUTPUT);
  pinMode(POSTN_LED, OUTPUT);

  // Wifi connection
  byte mac[6];
  WiFi.macAddress(mac);
  // IPs are static, DHCP does not look easy with arduino
  // My static IP
  IPAddress ip = getIp(mac);
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
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
  Serial.print(" Starting to the motor for ");
  Serial.println(stepsNumber);
  Serial.println(" steps");

  // how do we know the direction ?
  // 0 = close = downStep
  // 1 = open = upStep
  // We have to store precedent position
  if (nextPosition > 0 && nextPosition < 1) {
    if (previousPosition < nextPosition ) { // Open

    } else { // Close

    }
    previousPosition = nextPosition;
  }

}
