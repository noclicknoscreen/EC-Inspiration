/*
   OSC File :
   1/ Connect into UDP via Wifi
   2/ Send OSC Command
*/

#include <Arduino.h>
//#include "NCNS-ArduinoTools.h"

#define CTRL_LED 13

// Wifi Dependencies ----------------------------
#include <WiFiLink.h>
#include <WiFiUdp.h>

// OSC Dependencies -----------------------------
#include <OSCMessage.h>

// -------------------------------------------------
// Wifi Section
// -------------------------------------------------
char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                    // your network password

WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

// -------------------------------------------------
// OSC Section
// -------------------------------------------------
//destination IP
IPAddress outIp(192, 168, 2, 31);
const unsigned int outPort = 2391;

// -------------------------------------------------
// Specials Eurocave Section
// -------------------------------------------------
#define X_BASE_COL 2
#define S_BASE_COL 10
#define M_BASE_COL 18
#define L_BASE_COL 26

#define IN_BONUS  0
#define OUT_BONUS 1

//int xColBonus, sColBonus, mColBonus, lColBonus;
int numVignette;

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

void OscInit() {

  randomSeed(analogRead(0));

  // IPs are static, DHCP does not look easy with arduino
  // My static IP
  IPAddress ip = IPAddress(192, 168, 2, 11);

  // WiFi Init --------------------------------
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC Address: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);

  Serial.print("Connecting to SSID [");
  Serial.print(ssid);
  Serial.print("] pass [");
  Serial.print(pass);
  Serial.println("]");

  WiFi.config(ip);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Blink Led
    ledBlink(CTRL_LED, 50);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);

  // Init the column bonuses
  /*
    xColBonus = random(4);
    sColBonus = random(4);
    mColBonus = random(4);
    lColBonus = random(4);
    numVignette = random(3);
  */
}

/*
   Check and comment via Serial
*/
void wifiCheck() {

  // Control the connection (led #0)
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    ledBlink(CTRL_LED, 50);
  }

}

void sendIt(String _address, uint8_t _intValue) {

  char _addr[255];
  _address.toCharArray(_addr, 255);

  Serial.print("Sending message Addr=");
  Serial.print(_address);
  Serial.print(" Value=");
  Serial.print(String(_intValue, DEC));
  Serial.print(" To : ");
  Serial.print(outIp);
  Serial.print(",");
  Serial.println(outPort);

  OSCMessage msg(_addr);

  msg.add(_intValue);

  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message

}


