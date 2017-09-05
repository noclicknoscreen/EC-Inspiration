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
// Motor driving files
// --------------------------------------------------------------------------------------
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// --------------------------------------------------------------------------------------
// Generic helping functions
#include <NCNS-ArduinoTools.h>

// --------------------------------------------------------------------------------------
// Eeprom features
// --------------------------------------------------------------------------------------
#include <EEPROM.h>

// --------------------------------------------------------------------------------------
// Global declarations

#define ERROR_LED         0
#define POSTN_LED         2 //4
#define MANUAL_UP_PIN     0 //5
#define MANUAL_DOWN_PIN   12 // OK
#define MANUAL_RESET_PIN  14 //15 //0

#define NUMBER_OF_FEATHERS 4
#define MONITORING_IP IPAddress(192, 168, 2, 40)
#define MONITORING_PORT 12000

#define STEPPING_ADJUST 20 // Number of steps used in manually adjusting
#define CLOSED_STATE 0
#define OPENED_STATE 1
#define MOTORDEBUG

// Structure to declare all the feathers of the install
// This helps a lot keeping this code unique for all devices.
typedef struct {
  String name;
  String mac_address;
  IPAddress ip;
  int eeprom_addr;  // EEPROM Address to store/read value of totalSteps after
  int totalSteps;
  int stepsByRevolution;
  int speed;
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
  feathers[0].eeprom_addr = 0;
  feathers[0].totalSteps = 0;
  feathers[0].stepsByRevolution = 200; // nb de pas par tour
  feathers[0].speed = 100000; // 82 rpm

  feathers[1].name = "Feather 2 - Volet de la Cave Z";
  feathers[1].mac_address = "5C:CF:7F:3A:1B:8E";
  feathers[1].ip = IPAddress(192, 168, 2, 13);
  feathers[1].eeprom_addr = 0;
  feathers[1].totalSteps = 0;
  feathers[1].stepsByRevolution = 200; // nb de pas par tour
  feathers[1].speed = 100000; // 82 rpm

  feathers[2].name = "Feather 3 - Volet de la Cave Y";
  feathers[2].mac_address = "5C:CF:7F:3A:39:41";
  feathers[2].ip = IPAddress(192, 168, 2, 14);
  feathers[2].eeprom_addr = 0;
  feathers[2].totalSteps = 0;
  feathers[2].stepsByRevolution = 200; // nb de pas par tour
  feathers[2].speed = 100000; // 82 rpm

  feathers[3].name = "Feather 4 - Volet de la Cave X";
  feathers[3].mac_address = "5C:CF:7F:3A:2D:73";
  feathers[3].ip = IPAddress(192, 168, 2, 15);
  feathers[3].eeprom_addr = 0;
  feathers[3].totalSteps = 0;
  feathers[3].stepsByRevolution = 200; // nb de pas par tour
  feathers[3].speed = 100000; // 82 rpm

}

// --------------------------------------------------------------------------------------
// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x60); // Default address, no jumper, no stacking
// Generic motor connected. We need to adjust step number when we will receive the motor
// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);
float currentPosition = 0.0; // We suppose that the store is closed at initial state.
int currentState = CLOSED_STATE; // We suppose that the store is closed at initial state.

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
unsigned int receivedPosition = 0; // LOW means led is *on*
boolean ignore_osc_messages = false; // if false, then listen and read OSC event. Otherwise waiting for finish movement

/*************************************************
   Writing a float in EEPROM
 **************************************************/
void eeprom_write(int addr, float f) {
  unsigned char *buf = (unsigned char*)(&f);
  for ( int i = 0 ; i < (int)sizeof(f) ; i++ ) {
    EEPROM.write(addr + i, buf[i]);
  }
  EEPROM.commit();
}

/*************************************************
   Reading a float from EEPROM
 **************************************************/
float eeprom_read(int addr) {
  float f;
  unsigned char *buf = (unsigned char*)(&f);
  for ( int i = 0 ; i < (int)sizeof(f) ; i++ ) {
    buf[i] = EEPROM.read(addr + i);
  }
  return f;
}


/*-----------------------------------------------*
     Handle to /close, /open and /position (or /)
  -----------------------------------------------*/
void handleRoot() {
  Serial.println("Requested '/'");
  String content = "{ value : 0.0, ";
  content += "message : '";
  content += featherInfo();
  content += "\n\n/open to open the store.";
  content += "\n/close to close the store.";
  content += "\n/pause to pause the movement.";
  content +=  "' }";
  server.send(200, "text/json", content);
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
  openStore(feathers[featherId].totalSteps);
}

void handleClose() {
  Serial.println("Requested '/close'");
  server.send(200, "text/json", "{ value : '', message : 'Closing the store...' }");
  closeStore(feathers[featherId].totalSteps);
}

void handlePause() {
  Serial.println("Requested '/pause'");
  server.send(200, "text/json", "{ value : '', message : 'The store is paused (TODO) ' }");
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

String humanReadableIp(IPAddress ip) {
  return String(ip[0]) + String(".") + String(ip[1]) + String(".") + String(ip[2]) + String(".") + String(ip[3]);
}

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
  str += "\nTotal Steps Number : ";
  str += feathers[featherId].totalSteps;
  str += "\nSpeed : ";
  str += feathers[featherId].speed;
  str += " rpm";
  str += "\n----------------------------------------\n";
  return str;
}

/*
   Reading OSC Bundles, and treat them with an callback
*/
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


/*
   Reading OSC Bundles on the network
*/
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

/*-----------------------------------------------
                   S E T U P
  ----------------------------------------------*/
void setup() {
  // Serial
  Serial.begin(115200);
  Serial.println("");

  // Feathers definitions
  initFeathers();

  // Pins definitions
  pinMode(ERROR_LED, OUTPUT);
  pinMode(POSTN_LED, OUTPUT);

  pinMode(MANUAL_UP_PIN, INPUT);    // For manual opening
  digitalWrite(MANUAL_UP_PIN, HIGH);
  pinMode(MANUAL_DOWN_PIN, INPUT);  // For manual closing
  digitalWrite(MANUAL_DOWN_PIN, HIGH);
  pinMode(MANUAL_RESET_PIN, INPUT);  // For manual reseting of the total Steps
  digitalWrite(MANUAL_RESET_PIN, HIGH);

  // Who Am I ?
  featherId = guessFeather();

  // Read stored values of totalStep in EEPROM
  Serial.print("Reading total steps for feather #");
  Serial.print(featherId);
  Serial.println(" in EEPROM...");
  EEPROM.begin(128);  // 0 to 4096
  int val = int(eeprom_read(feathers[featherId].eeprom_addr));
  Serial.print("Read : ");
  Serial.println(val);
  // If eeprom has never been initialized, it may have a big ramdom value, so set it to 0.
  if (val > 999999) {
    val = 0;
  }
  feathers[featherId].totalSteps = val;
  Serial.print(featherInfo());

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

  // Stepper ignition
  Serial.println("init Stepper with 1.6kHz frequency...");
  AFMS.begin();  // create with the default frequency 1.6KHz
  Serial.print("Set speed to ");
  Serial.print(feathers[featherId].speed);
  Serial.println(" rpm");
  myMotor->setSpeed(feathers[featherId].speed); // We probably need to adjust speed later in the code to keep the same time between each store.
  Serial.println("Stepper initialized !");
  Serial.println("");

  // Close the store at startup.

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

/*-----------------------------------------------
                   L O O P
  ----------------------------------------------*/
void loop() {

  // Setting up totalSteps by manual motor driving
  if ( digitalRead(MANUAL_UP_PIN ) == LOW) {
    // 1. running motor step by step
    openStore(STEPPING_ADJUST);
    // 2. Saving totalSteps into EEPROM
    eeprom_write(feathers[featherId].eeprom_addr, feathers[featherId].totalSteps + STEPPING_ADJUST);
    feathers[featherId].totalSteps += STEPPING_ADJUST;
    Serial.print(" ------>  Set total steps to : ");
    Serial.println(feathers[featherId].totalSteps);
  }

  // Closing manually the door
  if ( digitalRead(MANUAL_DOWN_PIN) == LOW) {
    closeStore(STEPPING_ADJUST);
    Serial.print(" ------>  Closing store step by step, (");
    Serial.print(STEPPING_ADJUST);
    Serial.println(")");
  }

  // Reset Total Steps manually
  if ( digitalRead(MANUAL_RESET_PIN) == LOW ) {
    Serial.println(" ------>  Reset total steps to 0");
    eeprom_write(feathers[featherId].eeprom_addr, 0);
    feathers[featherId].totalSteps = 0;
    digitalWrite(MANUAL_RESET_PIN, HIGH);
    delay(1000);
  }

  // Control the connection (led #0)
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    errorBlink(ERROR_LED, 100);
  } else {

    if ( feathers[featherId].totalSteps == 0 ) {

      Serial.println("Total steps not configured ! Please push the green button until the store is correctly opened...");
      errorBlink(ERROR_LED, 500);

    } else {

      // Handle webserver
      server.handleClient();

      ledBlink(ERROR_LED, 1000);

      // Then wait for OSC if all movements are achieved
      if (!ignore_osc_messages) {
        readOSCBundle();
      }

    }
  }
}

/*
   Function callback, called at every OSC bundle received
*/
void positionChange(OSCMessage &msg) {
  // Possibly issue onto Millumin, so constrain the values
  // This should be 0.0 or 1.0
  float nextPosition = constrain(msg.getFloat(0), 0.0, 1.0);

  receivedPosition = 255 * nextPosition;
  analogWrite(POSTN_LED, receivedPosition);

  Serial.print("/position: ");
  Serial.print(nextPosition);
  Serial.print(", Running to the motor for ");
  Serial.print(feathers[featherId].totalSteps);
  Serial.print(" steps. ");
  Serial.print("This should take about ");
  Serial.print(60 * feathers[featherId].totalSteps / feathers[featherId].stepsByRevolution / feathers[featherId].speed);
  Serial.println(" seconds.");

  ignore_osc_messages = true;
  // We have to store precedent position
  if (nextPosition == 0.0 || nextPosition == 1.0) {
    if (currentPosition < nextPosition ) {
      // close all others stores
      for (int i = 0; i < NUMBER_OF_FEATHERS; i++) {
        if (i != featherId) {
          sendOSCBundle(IPAddress(feathers[i].ip), localPort, "/position", 0.0);
        }
      }
      // Open
      openStore(feathers[featherId].totalSteps);
      sendOSCBundle(MONITORING_IP, MONITORING_PORT, "/feedback/position/" + featherId, 1.0);
    } else {
      // Close
      closeStore(feathers[featherId].totalSteps);
      sendOSCBundle(MONITORING_IP, MONITORING_PORT, "/feedback/position/" + featherId, 0.0);
    }
    currentPosition = nextPosition;
    ignore_osc_messages = false;

  } else { // Not the value we were waiting for.
    ledBlink(ERROR_LED, 200);
    Serial.print("The value was not suitable (");
    Serial.print(nextPosition);
    Serial.println("). This should be 0.0 or 1.0.");
  }
}

/*
   Ouverture du store
*/
void openStore(int steps) {
  if (currentState != OPENED_STATE ) {
    Serial.print("motor runs for ");
    Serial.print(steps);
    Serial.println(", FORWARD.");
    myMotor->setSpeed(feathers[featherId].speed);
    myMotor->step(steps, FORWARD, DOUBLE);
    currentState = OPENED_STATE;
  } else {
    Serial.println("The store is already opened !");
  }
}

/*
   Fermeture du store
*/
void closeStore(int steps) {
  if (currentState != CLOSED_STATE ) {
    Serial.print("motor runs for ");
    Serial.print(steps);
    Serial.println(", BACKWARD.");
    myMotor->setSpeed(feathers[featherId].speed);
    myMotor->step(steps, BACKWARD, DOUBLE);
    // Release motor : no need to maintain couple
    myMotor->release();
    currentState = CLOSED_STATE;
  } else {
    Serial.println("The store is already closed !");
  }
}

