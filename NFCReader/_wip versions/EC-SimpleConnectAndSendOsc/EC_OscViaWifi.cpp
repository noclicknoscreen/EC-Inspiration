

#include "EC_OscViaWifi.h"

void OscViaWifi::wifiSetup(String _ssid, String _pass) {

  char* ssid;     //  your network SSID (name)
  char* pass;        // your network password

  _ssid.toCharArray(ssid, _ssid.length());
  _pass.toCharArray(pass, _pass.length());

  //Check if communication with the wifi module has been established
  if (WiFi.status() == WL_NO_WIFI_MODULE_COMM) {
    Serial.println("Communication with WiFi module not established.");
    while (true);// don't continue:
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA");
    Serial.print(" SSID: ");
    Serial.print(_ssid);
    Serial.print(" PASS: ");
    Serial.println(_pass);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  // Get the mac address
  WiFi.macAddress(myMac);
}


void OscViaWifi::udpSetup(int _localPort) {

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (Udp.begin(_localPort) > 0) {
    port = _localPort;
    Serial.println("Connected to server.");
  }

}

void OscViaWifi::udpLoop(char _tagSize) {

  char packetBuffer[255]; //buffer to hold incoming packet
  //char  ReplyBuffer[] = "acknowledged";       // a string to send back


  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(_tagSize);
    Udp.endPacket();
  }

}

void OscViaWifi::printInfos() {

  printCurrentNet();
  printWifiData();

}

void OscViaWifi::printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress myIp = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(myIp);

  // print your MAC address:
  Serial.print("MAC address: ");
  Serial.print(myMac[5], HEX);
  Serial.print(":");
  Serial.print(myMac[4], HEX);
  Serial.print(":");
  Serial.print(myMac[3], HEX);
  Serial.print(":");
  Serial.print(myMac[2], HEX);
  Serial.print(":");
  Serial.print(myMac[1], HEX);
  Serial.print(":");
  Serial.println(myMac[0], HEX);

}

void OscViaWifi::printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}



