/*

 This example connects to a WPA encrypted Wifi network.
 Then it prints the  MAC address of the Wifi module,
 the IP address obtained, and other network details.

 Circuit:
 * Arduino Primo or STAR OTTO or Uno WiFi Developer Edition (with WiFi Link firmware running)

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */

#include "EC_OscViaWifi.h"

OscViaWifi oscViaWifi;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  oscViaWifi.wifiSetup("InspirationHub", "Inspiration");
  oscViaWifi.udpSetup(2390);
  
}

void loop() {
  // check the network connection once every 10 seconds:
  delay(1000);
  oscViaWifi.udpLoop('x');
  
}


