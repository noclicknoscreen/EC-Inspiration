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
char ssid[] = "linksys-MedenAgan";          // your network SSID (name)
char pass[] = "Edwood72";                    // your network password

WiFiUDP Udp;
const unsigned int localPort = 2390;        // local port to listen for UDP packets (here's where we send the packets)

// -------------------------------------------------
// OSC Section
// -------------------------------------------------
//destination IP
IPAddress outIp(192, 168, 2, 31);
const unsigned int outPort = 5000;

// -------------------------------------------------
// Specials Eurocave Section
// -------------------------------------------------
#define X_BASE_COL 2
#define S_BASE_COL 10
#define M_BASE_COL 18
#define L_BASE_COL 26

#define IN_BONUS  0
#define OUT_BONUS 1

int xColBonus, sColBonus, mColBonus, lColBonus;

void OscInit() {

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
  
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Blink Led
    errorBlink(CTRL_LED, 50);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);

  // Init the column bonuses
  xColBonus = random(4);
  sColBonus = random(4);
  mColBonus = random(4);
  lColBonus = random(4);

}

void wifiCheck() {

  // Control the connection (led #0)
  if (WiFi.status() != WL_CONNECTED) {
    // not connected => Message + Blink Short
    Serial.println("Wifi Not Connected :(");
    errorBlink(CTRL_LED, 50);
  }

}

int incBonus(int _bonus){
  // Increment
  _bonus++;
  // If you go too far, come back !
  if(_bonus >= 4){
     _bonus = 0;
  }

  return _bonus;
  
}

int getNextBonus(char _tag) {

  if (_tag == 'X') {
    // -----------------------------------
    xColBonus = incBonus(xColBonus);
    return xColBonus;
  } else if (_tag == 'S') {
    // -----------------------------------
    sColBonus = incBonus(sColBonus);
    return sColBonus;
  } else if (_tag == 'M') {
    // -----------------------------------
    mColBonus = incBonus(mColBonus);
    return mColBonus;
  } else if (_tag == 'L') {
    // -----------------------------------
    lColBonus = incBonus(lColBonus);
    return lColBonus;
  }

  return 0;

}

int getCurrentBonus(char _tag) {

  if (_tag == 'X') {
    // -----------------------------------
    return xColBonus;
  } else if (_tag == 'S') {
    // -----------------------------------
    return sColBonus;
  } else if (_tag == 'M') {
    // -----------------------------------
    return mColBonus;
  } else if (_tag == 'L') {
    // -----------------------------------
    return lColBonus;
  }

  return 0;

}

int getColumnIdx(char _tag) {

  int columnIdx = -1;

  if (_tag == 'X') {
    // -----------------------------------
    columnIdx = X_BASE_COL;
  } else if (_tag == 'S') {
    // -----------------------------------
    columnIdx = S_BASE_COL;
  } else if (_tag == 'M') {
    // -----------------------------------
    columnIdx = M_BASE_COL;
  } else if (_tag == 'L') {
    // -----------------------------------
    columnIdx = L_BASE_COL;
  }

  return columnIdx;

}

void sendIt(String _address, int _intValue) {

  char _addr[255];
  _address.toCharArray(_addr, 255);

  Serial.print("Sending message Addr=");
  Serial.print(_address);
  /*
    Serial.print(" len=");
    Serial.print(_address.length());
    Serial.print(" Char Addr=");
    Serial.print(_addr);
  */
  Serial.print(" Value=");
  Serial.println(String(_intValue));

  OSCMessage msg(_addr);

  msg.add((uint8_t) _intValue);

  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message

}

void sendIt(String _address, String _strValue) {

  char _addr[255];
  _address.toCharArray(_addr, 255);

  Serial.print("Sending message Addr=");
  Serial.print(_address);
  /*
    Serial.print(" len=");
    Serial.print(_address.length());
    Serial.print(" Char Addr=");
    Serial.print(_addr);
  */
  Serial.print(" Value=");
  Serial.println(_strValue);

  OSCMessage msg(_addr);

  msg.add(_strValue);

  Udp.beginPacket(outIp, outPort);
  msg.send(Udp); // send the bytes to the SLIP stream
  Udp.endPacket(); // mark the end of the OSC Packet
  msg.empty(); // free space occupied by message

}

// ------------------------------------------------------------
// Sending TAG
// To send a tag :
// 1st step : get the base column index
// 2st step : set a bonus
//    - bonus to In animation = 0
//    - bonus to Out animation = 1
// 3rd step : get a second bonus value determined by the decorum style you want
// ------------------------------------------------------------

void sendTag(char _tag, int _inOutBonus, int _colBonus) {

  int columnIdx = getColumnIdx(_tag);

  Serial.print("Sending Message, Tag=[");
  Serial.print(_tag);
  Serial.print("] , colBase=[");
  Serial.print(String(columnIdx));
  Serial.print("], inOutBonus=[");
  Serial.print(String(_inOutBonus));
  Serial.print("], colBonus=[");
  Serial.print(String(_colBonus));
  Serial.println("]");
  
  if (columnIdx >= 1) {
    // Idx is right, send it !
    sendIt("/millumin/action/launchColumn", _inOutBonus + 2*_colBonus + columnIdx);
  } else {
    // not right, print a tip
    Serial.print("Column Idx is out of range, probably wrong tag (");
    Serial.print(_tag);
    Serial.print(",");
    Serial.print(")");
    Serial.println();
  }

}

