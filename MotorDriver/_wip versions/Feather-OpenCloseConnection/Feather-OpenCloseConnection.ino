
/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid     = "InspirationHub";
const char* password = "Inspiration";

// UDP
WiFiUDP Udp;
#define localPort 2390      // local port to listen on

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.print("Connecting to SSID: ");
  Serial.print(ssid);
  Serial.print(" PASS: ");
  Serial.print(password);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // if you get a connection, report back via serial:
  Udp.begin(localPort);

}

int value = 0;

void loop() {

  delay(100);
  ++value;
  /*
  Serial.print(millis());
  Serial.print(" : ");
  Serial.print(WiFi.localIP());
  Serial.print(" : ");
  Serial.print("WiFi Status = ");
  Serial.print(WiFi.status());
  Serial.print(" (Status Waiting : ");
  Serial.print(WL_CONNECTED);
  Serial.print(")");
  Serial.println();
  */
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("DISCONNECTED");
  }
  
  // Read a packet
  UDPRead();
  // Write some message
  char message[] = "Who are u ?";
  UDPWrite(message);
  
}

void UDPRead() {

  char packetBuffer[255]; //buffer to hold incoming packet
  
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print(millis());
    Serial.print(" : ");
    Serial.print("Received packet of size ");
    Serial.print(packetSize);
    Serial.print(" From ");
    //IPAddress remoteIp = Udp.remoteIP();
    Serial.print(Udp.remoteIP());
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.print("Contents:");
    Serial.println(packetBuffer);
  }
  
}

void UDPWrite(char *message){

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket("192.168.2.131", 11999);
    Udp.write(message);
    Udp.endPacket();
    
}

