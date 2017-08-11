
/*---------------------------------------------------------------------------------------------

  Open Sound Control (OSC) library for the ESP8266

  Example for receiving open sound control (OSC) bundles on the ESP8266
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.

  This example code is in the public domain.

  --------------------------------------------------------------------------------------------- */
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <NCNS-ArduinoTools.h>

char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                    // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

#define ERROR_LED   0
#define POSTN_LED   4

OSCErrorCode error;
unsigned int receivedPosition = 0;              // LOW means led is *on*

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  pinMode(ERROR_LED, OUTPUT);
  pinMode(POSTN_LED, OUTPUT);

  byte mac[6];
  WiFi.macAddress(mac);
  IPAddress ip = getIp(mac);  // Node static IP
  // Update these with values suitable for your network.
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
    errorBlink(ERROR_LED, 100);
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


}

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
  if (strMac == "60:1:94:19:ec:a8") {
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

void positionChange(OSCMessage &msg) {
  // Possibly issue onto Millumin, so constrain the values
  float msgContent = constrain(msg.getFloat(0), 0.0, 1.0);

  receivedPosition = 255 * msgContent;
  analogWrite(POSTN_LED, receivedPosition);

  Serial.print("/position: ");
  Serial.print(msgContent);
  Serial.print(" receivedPosition : ");
  Serial.println(receivedPosition);

}

void loop() {

  // Control the connection (led #0)
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    errorBlink(ERROR_LED, 100);

  } else {

    errorBlink(ERROR_LED, 1000);

    // Then wait for OSC
    OSCBundle bundle;
    int size = Udp.parsePacket();

    if (size > 0) {
      Serial.print(millis());
      Serial.print(" : ");
      Serial.println("OSC Packet as Bundle Received");

      while (size--) {
        bundle.fill(Udp.read());
      }

      if (!bundle.hasError()) {
        bundle.dispatch("/position", positionChange);
      } else {
        error = bundle.getError();
        Serial.print("error: ");
        Serial.println(error);
        // not connected => Message + Blink Lon
        errorBlink(ERROR_LED, 200);

      }
    }
  }
}



