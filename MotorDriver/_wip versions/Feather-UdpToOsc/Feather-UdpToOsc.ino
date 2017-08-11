
/*
    This sketch sends data via HTTP GET requests to data.sparkfun.com service.

    You need to get streamId and privateKey at data.sparkfun.com and paste them
    below. Or just customize this script to talk to other HTTP servers.

*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SimpleOSC.h>

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

  uint8_t packetBuffer[255]; //buffer to hold incoming packet
  String  strBuffer;

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

    // Print rough Content
    Serial.print("Content as Int :");
    for (int idxBuff = 0; idxBuff < len; idxBuff++) {
      Serial.print(String(packetBuffer[idxBuff], HEX));
      strBuffer += (char)packetBuffer[idxBuff];
    }
    Serial.println();

    Serial.print("Content as String :");
    Serial.println(strBuffer);
    if (strBuffer.startsWith("#bundle")) {
      // OSC Datas are in a bundle
      Serial.print(" It's a bundle :");

      String tag = strBuffer.substring(0, 7);
      uint8_t timeTag[8];
      memcpy(timeTag, packetBuffer, 8);

      char *message;
      memcpy(message, packetBuffer, 8);
      
    } else {
      // Print decoded message
      OSCMessage m;
      OSCDecoder decoder;

      // Blink as Message Received ----------------------------------------
      //analogWrite(LED_BUILTIN, packetBuffer[len - 1]);

      // Decode Message ----------------------------------------
      // SO Read the osc to get the color
      decoder.decode(&m, packetBuffer);
      String address = m.getOSCAddress();

      Serial.print("OSC Adress : ");
      Serial.print(m.getOSCAddress());
      Serial.print(" OSC Args : ");
      Serial.print(m.getArgsNum());
      Serial.print(" : ");
      Serial.print(" OSC Arg Tag : ");
      Serial.print(m.getArgTypeTag(0));
      Serial.print(" : ");

      for (int idxArgs = 0; idxArgs < m.getArgsNum(); idxArgs++) {

        Serial.print("[");
        Serial.print(idxArgs);
        Serial.print("]");
        Serial.print(":");

        if (m.getArgTypeTag(idxArgs) == kTagInt32) {
          Serial.print(" It's an int");

        } else if (m.getArgTypeTag(idxArgs) == kTagFloat) {
          Serial.print(" It's a float");

        } else if (m.getArgTypeTag(idxArgs) == kTagString) {
          Serial.print(" It's a string");
          //get 3rd argument(string)
          int strSize = m.getArgStringSize(idxArgs);
          char tmpStr[strSize]; //string memory allocation
          m.getArgString(idxArgs, tmpStr);
          // Print
          Serial.print(tmpStr);
          Serial.print(" ");

        } else if (m.getArgTypeTag(idxArgs) == kTagBlob) {
          Serial.print(" It's a blob");
          OSCBlob b;

        } else {
          Serial.print(" It's unknown :(");

        }
      }
    }
    Serial.println();

    // NEXT COMES SPECIFIC TREATMENT FOR Motor Driving
    /*
      if (address.startsWith("/animation")) {
      decodeAnimation(m);
      } else {
      // UNKNOWN --------------------
      }
    */
  }

}

void UDPWrite(char *message) {

  // send a reply, to the IP address and port that sent us the packet we received
  Udp.beginPacket("192.168.2.131", 11999);
  Udp.write(message);
  Udp.endPacket();

}

