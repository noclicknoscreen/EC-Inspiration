#include <Arduino.h>
#include <NCNS-ArduinoTools.h>

#define CTRL_LED 13

// NFC Dependencies ----------------------------
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

// -------------------------------------------------
// NFC Section
// -------------------------------------------------
PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
// Reading of the tag
//#define unknownTag  '?'
#define noTag       '#'

char lastTag = noTag;

void NFCInit() {

  // NFC Init -------------------------------------
  Serial.println("NDEF Writer");
  nfc.begin();

}


// ------------------------------------------------------------
// Reads the tag and return the size of
//
// Record 0 : is the name
// Record 1 : is timeStamp (millis :/)
// Record 2 : is the size expected
// ------------------------------------------------------------
char readSizeAsChar() {

  NfcTag tag = nfc.read();

  if (tag.hasNdefMessage()) // every tag won't have a message
  {

    NdefMessage message = tag.getNdefMessage();

    int recordCount = message.getRecordCount();

    // Trapping unexpected behaviors
    if (recordCount == 0) {
      Serial.print("No Record found.");
      return 'n';
    }

    if (recordCount >= 2) {

      // So idx seems good, let's get it !
      NdefRecord record = message.getRecord(2);

      int payloadLength = record.getPayloadLength();
      byte payload[payloadLength];
      record.getPayload(payload);

      // Force the data into a String (might work depending on the content)
      // Real code should use smarter processing
      String payloadAsString = "";
      for (int c = 0; c < payloadLength; c++) {
        payloadAsString += (char)payload[c];
      }

      char result = payloadAsString.charAt(1);
      //byte byteResult = payloadAsString.getBytesAt(1);

      Serial.print("NDEF Record : ");
      Serial.print("Payload (as String):");
      Serial.print(payloadAsString);
      Serial.print(" Char 0 :");
      Serial.print(String(result));
      Serial.print(" Char 0 as Int:");
      Serial.print(String(result, HEX));
      Serial.println();

      return result;

    } else {
      return 'e';
    }

  }
}


char nfcGetNewTag() {

  char newTag = noTag;

  if (nfc.tagPresent()) {
    newTag = readSizeAsChar();
  }

  return newTag;
  
}
