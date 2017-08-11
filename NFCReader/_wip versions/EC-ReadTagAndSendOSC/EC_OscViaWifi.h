
/*
   Wrapper to connect and send OSC

   Author : Sébastien Albert
   Created : 2017/07/21
   Updated : 2017/07/24

   Version : 1.2
   
*/

#ifndef EC_OSCViaWifi_h
#define EC_OSCViaWifi_h

#include <Arduino.h>
#include <WiFiLink.h>
#include <WiFiUdp.h>
//#include "ArdOSC.h"

class OscViaWifi {

  private:
    //    char ssid[] = "InspirationHub";     //  your network SSID (name)
    //    char pass[] = "Inspiration";        // your network password
    //int status = WL_IDLE_STATUS;        // the Wifi radio's status

    // UDP members
    byte myMac[6];
    int port = 0;
    WiFiUDP Udp;
    
  public:
    void errorBlink(unsigned int delayMs);
    void wifiSetup(String _ssid, String _pass);
    
    void udpSetup(int _localPort);
    void udpLoop(char _tagSize);
    void udpRead();
    void udpSend(char _tagSize);
    
    void printInfos();
    
  private:
    void printWifiData();
    void printCurrentNet();

};

#endif

