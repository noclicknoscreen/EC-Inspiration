#include "Osc.h"
#include "NFC.h"

#define CTRL_LED 13

void setup() {
  Serial.begin(115200);
  pinMode(CTRL_LED, OUTPUT);

  NFCInit();
  OscInit();

}

void loop() {

  wifiCheck();

  char currentTag = nfcGetNewTag();

  // --------------------------------------------------
  // IN = First, I had no tag and I put it in
  // --------------------------------------------------
  if (lastTag == noTag && currentTag != noTag) {
    // -------------------------------------------
    Serial.print("Brand New Tag : [");
    Serial.print(String(currentTag));
    Serial.println("]");
    sendTagIn(currentTag);
    lastTag = currentTag;
  }

  // --------------------------------------------------
  // OUT = Second, I had a tag and put it out
  // --------------------------------------------------
  else if (lastTag != noTag && currentTag == noTag) {
    // -------------------------------------------
    Serial.print("Removed old Tag : [");
    Serial.print(String(lastTag));
    Serial.println("]");
    sendTagOut(lastTag);
    lastTag = currentTag;
  }

  else {

    Serial.print(millis());
    Serial.println(" : Place your tag. Waiting for reading.");
    //  Serial.println("1 Second wait.");

  }

  // Wait a bit
  delay(50);

}





