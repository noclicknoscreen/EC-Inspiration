#include <Arduino.h>
#include <NCNS-ArduinoTools.h>

#define CTRL_LED 13

// Wifi Dependencies ----------------------------
#include <WiFiLink.h>
#include <WiFiUdp.h>

// OSC Dependencies -----------------------------
#include <OSCMessage.h>

// -------------------------------------------------
// Wifi Section
// -------------------------------------------------
char ssid[] = "InspirationHub";          // your network SSID (name)
char pass[] = "Inspiration";                    // your network password

WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

// -------------------------------------------------
// OSC Section
// -------------------------------------------------
//destination IP
IPAddress outIp(192, 168, 2, 131);
const unsigned int outPort = 5000;

void OscInit() {

  // WiFi Init --------------------------------
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Blink Led
    errorBlink(CTRL_LED, 50);
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);

}

void wifiCheck() {

  // Control the connection (led #0)
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    errorBlink(CTRL_LED, 50);
  }

}



void sendIt(String _address, int _value) {

  char _addr[255];
  _address.toCharArray(_addr, 255);

  Serial.print("Sending message Addr=");
  Serial.print(_address);
  Serial.print(" len=");
  Serial.print(_address.length());
  Serial.print(" Char Addr=");
  Serial.println(_addr);

  OSCMessage msg(_addr);

  msg.add((uint8_t) _value);

  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message

}

void sendTagIn(char _tag) {
  Serial.println("Sending IN !!!!!!!!!!!!!!!!!!!");
  
  String inAddress;
  inAddress += "/tag";
  inAddress += _tag;
  inAddress += "/In";

  sendIt(inAddress, 1);
  delay(1000);
  sendIt(inAddress, 0);
  
}

void sendTagOut(char _tag) {
  Serial.println("Sending OUT !!!!!!!!!!!!!!!!!!!");
  
  String outAddress;
  outAddress += "/tag";
  outAddress += _tag;
  outAddress += "/Out";

  sendIt(outAddress, 1);
  delay(1000);
  sendIt(outAddress, 0);

}
