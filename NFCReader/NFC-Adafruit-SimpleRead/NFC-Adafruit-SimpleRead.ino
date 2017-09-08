#include "NCNS-NFCWrapper.h"

/* -------------------------------------
    Author : Sebastien Albert
    Date created : 2017/09/05

    5/9/17 : First steps with NFC Wrapper
    
*/

NFCMifareWrapper NfcWrapper;

long nbReads = 0;

void setup() {

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif
  Serial.begin(115200);
  // put your setup code here, to run once:
  NfcWrapper.setup();
}

void loop() {
  // Read every 200 ms a tag !

  if (NfcWrapper.isTagPresent()) {

    // READ
    String valueString = NfcWrapper.readMifareBlock(4);
    Serial.println("-------------------------------------------------------");
    Serial.print("Sentence read on sector 4 : ");
    Serial.println(valueString);
    
    Serial.print(millis());
    Serial.print(" : Nb Reads=");
    Serial.println(String(++nbReads, DEC));
    Serial.println("-------------------------------------------------------");
  } else {
    Serial.println("Waiting for a tag ..............");
  }

  delay(200);

}
