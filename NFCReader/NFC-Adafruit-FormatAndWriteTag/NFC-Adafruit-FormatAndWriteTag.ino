#include "NCNS-NFCWrapper.h"

/* -------------------------------------
    Author : Sebastien Albert
    Date created : 2017/09/05

    5/9/17 : First steps with NFC Wrapper

*/

NFCMifareWrapper NfcWrapper;

void setup() {

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif
  Serial.begin(115200);

  // put your setup code here, to run once:
  NfcWrapper.setup();

}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Place your NDEF formatted Mifare Classic 1K card on the reader");
  Serial.println("and press any key to continue ...");

  // Wait for user input before proceeding
  while (!Serial.available());
  while (Serial.available()) Serial.read();

  NfcWrapper.formatMifare();

  // Wait a bit before trying again
  Serial.println("\n\nDone!");
  delay(1000);
  Serial.flush();
  while (Serial.available()) Serial.read();

}
