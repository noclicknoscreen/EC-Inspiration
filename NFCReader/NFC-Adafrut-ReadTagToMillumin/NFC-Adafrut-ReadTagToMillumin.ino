/*
   Written, maintained by : Sébastien Albert
   V 0.0.1
*/

#include "Osc.h"
#include "NFC.h"

#define CTRL_LED 13

void setup() {
  // Init the serial
  Serial.begin(115200);
  pinMode(CTRL_LED, OUTPUT);

  // Init the Shield
  NFCInit();

  // Init connection
  OscInit();

}

void loop() {

  // Check connection and talk about it
  wifiCheck();

  char currentTag = nfcGetNewTag();

  // --------------------------------------------------
  // IN = First, I had no tag and I put it in
  // --------------------------------------------------
  if (lastTag == TAGR_EMPTY && currentTag != TAGR_EMPTY) {
    // -------------------------------------------
    Serial.print("Brand New Tag : [");
    Serial.print(String(currentTag));
    Serial.println("]");
    sendTag(currentTag, IN_BONUS, getNextBonus(currentTag));
    lastTag = currentTag;
  }

  // --------------------------------------------------
  // OUT = Second, I had a tag and put it out
  // --------------------------------------------------
  else if (lastTag != TAGR_EMPTY && currentTag == TAGR_EMPTY) {
    // -------------------------------------------
    Serial.print("Removed old Tag : [");
    Serial.print(String(lastTag));
    Serial.println("]");
    sendTag(lastTag, OUT_BONUS, getCurrentBonus(lastTag));
    lastTag = currentTag;
  }

  // --------------------------------------------------
  // nothing, print a dot
  // --------------------------------------------------
  else {
    Serial.print(".");
  }
}
