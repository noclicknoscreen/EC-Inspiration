/*
   NFC File :
   1/ Read the tag, and talk into OSC
*/

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

#define TAGR_EMPTY 'e'

char lastTag = TAGR_EMPTY;

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

  // 'e' as Emtpy
  char result = TAGR_EMPTY;
  NfcTag tag;
  String uid;

  tag = nfc.read();

  uid = tag.getUidString();

  Serial.print("Read a tag [");
  Serial.print(tag.getTagType());
  Serial.print("] UID: ");
  Serial.println(tag.getUidString());

  // In case of emergency
  // X : EA 2F 0C 52
  // S : FA F3 EC A3
  // M : 9A 80 AF DD
  // L : DA C7 EA A3
  if (uid == "EA 2F 0C 52") {
    result = 'X';
  } else if (uid == "FA F3 EC A3") {
    result = 'S';
  } else if (uid == "9A 80 AF DD") {
    result = 'M';
  } else if (uid == "DA C7 EA A3") {
    result = 'L';
  }

  return result;

  //  if (tag.hasNdefMessage()) // every tag won't have a message
  //  {
  //
  //    NdefMessage message = tag.getNdefMessage();
  //
  //    int recordCount = message.getRecordCount();
  //
  //    // Trapping unexpected behaviors
  //    if (recordCount == 0) {
  //      Serial.print("No Record found.");
  //      result = TAGR_EMPTY;
  //
  //    } else if (recordCount >= 2) {
  //
  //      // So idx seems good, let's get it !
  //      NdefRecord record = message.getRecord(2);
  //
  //      int payloadLength = record.getPayloadLength();
  //      byte payload[payloadLength];
  //      record.getPayload(payload);
  //
  //      // Force the data into a String (might work depending on the content)
  //      // Real code should use smarter processing
  //      String payloadAsString = "";
  //      for (int c = 0; c < payloadLength; c++) {
  //        payloadAsString += (char)payload[c];
  //      }
  //
  //      result = payloadAsString.charAt(1);
  //      //byte byteResult = payloadAsString.getBytesAt(1);
  //      /*
  //        Serial.print("NDEF Record : ");
  //          Serial.print("Payload (as String):");
  //        Serial.print(payloadAsString);
  //        Serial.print(" Char 0 :");
  //        Serial.print(String(result));
  //        Serial.print(" Char 0 as Int:");
  //        Serial.print(String(result, HEX));
  //        Serial.println();
  //      */
  //      //return result;
  //
  //    }
  //  }
  //
  //  return result;

}


char nfcGetNewTag() {

  char newTag = TAGR_EMPTY;

  if (nfc.tagPresent()) {
    newTag = readSizeAsChar();
  }

  return newTag;

}
