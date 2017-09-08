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
  char readChar;
  String readString = "";

  Serial.println("Place your NDEF formatted Mifare Classic 1K card on the reader");
  Serial.println("and press any key to continue ...");

  // Wait for user input before proceeding
  while (!Serial.available());

  // --
  do {
    while (Serial.available() > 0) {
      // Concatenates every char
      readChar = Serial.read();
      readString += readChar;
    }
    // --
    //Serial.print("Char read : [");
    //Serial.print(readChar);
    //Serial.println("]");
    // --
  } while (readChar != '\n');

  if (NfcWrapper.isTagPresent()) {
    
    // 1/ FORMAT
    Serial.println("[ENTER] , now we format and write into the Tag :");
    Serial.print("Sentence written : ");
    Serial.println(readString);
    NfcWrapper.formatMifare();

    // 2/ WRITE
    NfcWrapper.writeMifareBlock(4, readString);

    // 3/ READ CHECK
    String valueString = NfcWrapper.readMifareBlock(4);
    Serial.print("Sentence read : ");
    Serial.println(valueString);
    
    // Wait a bit before trying again
    Serial.println("Done!");
    Serial.println("-----------------------------------------");
    Serial.println();
    Serial.println();

  } else {
    Serial.println("Waiting for a tag ..............");
  }

  delay(1000);

  // Then Flush
  Serial.flush();
  while (Serial.available()) Serial.read();

}
