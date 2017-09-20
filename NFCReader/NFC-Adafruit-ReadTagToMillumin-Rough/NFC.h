/*
   NFC File :
   1/ Read the tag, and talk into OSC
*/

#include <Arduino.h>
//#include "NCNS-ArduinoTools.h"

#define CTRL_LED 13

// NFC Dependencies ----------------------------
#include "NCNS-NFCWrapper.h"

NFCMifareWrapper NfcWrapper;

// Reading of the tag
#define TAGR_EMPTY 'e'

char lastTag = TAGR_EMPTY;

void NFCInit() {
  // NFC Init -------------------------------------
  Serial.println("NDEF Writer");
  NfcWrapper.setup();
}


// ------------------------------------------------------------
// Reads the tag and return the size of
// ------------------------------------------------------------
char nfcGetNewTag() {
  
  char newTag = TAGR_EMPTY;
  
  if (NfcWrapper.isTagPresent()) {  
    String valueString = NfcWrapper.readMifareBlock(4);
    if (valueString.length() > 0) {
      newTag = valueString.charAt(0);
    }
  }

  return newTag;
}
