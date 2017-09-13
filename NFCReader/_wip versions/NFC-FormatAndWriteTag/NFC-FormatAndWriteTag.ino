
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);

void setup() {
  Serial.begin(9600);
  Serial.println("NDEF Writer");
  nfc.begin();
}

void loop() {

  Serial.println("Place your tag. It will be formatted as NDEF");
  if (nfc.tagPresent()) {
    bool success;

    // ---------------------------------------------
    // First clean it.
    success = nfc.clean();
    if (success) {

      // Then format.
      success = nfc.format();
      if (success) {
        Serial.println("Success, tag cleaned and formatted as NDEF.");
      } else {
        Serial.println("Format failed.");
      }

    } else {
      Serial.println("Clean failed.");
    }
    // ---------------------------------------------

    // ---------------------------------------------
    // Great, we can write ----
    Serial.println("Waiting for an order : 1 (XS Tag), 2 (S Tag), 3 (M Tag) or 4 (XL Tag)");
    bool orderDone = false;
    String inputString = "";

    while (!orderDone) {
      // if there's any serial available, read it:
      while (Serial.available() > 0) {
        // look for the newline. That's the end of your
        // sentence:
        char inChar = Serial.read();
        inputString += inChar;
        if (inChar == '\n') {
          orderDone = true;
        }
      }
    }

    if (orderDone == true) {
      writeOrder(inputString.toInt());// Write
      readTag();// Then read it to confirm

      // Finaly wait
      Serial.println("2 Seconds wait.");
      delay(2000);
    }
  }

  Serial.println("1 Second wait.");
  delay(1000);

}

void writeOrder(int order) {
  
  Serial.print("Order is : ["); Serial.print(String(order)); Serial.println("]");
  
  NdefMessage message = NdefMessage();
  bool success;

  message.addUriRecord("Eurocave - Inspiration");
  
  if (order == 1) {
    message.addUriRecord("X");
  } else if (order == 2) {
    message.addUriRecord("S");
  } else if (order == 3) {
    message.addUriRecord("M");
  } else if (order == 4) {
    message.addUriRecord("L");
  } else {
    Serial.println("Wrong order.");
  }

  success = nfc.write(message);
  if (success) {
    Serial.println("Success, tag written.");
  } else {
    Serial.println("Write failed.");
  }

}

void readTag() {

  NfcTag tag = nfc.read();
  Serial.print("UID: "); Serial.println(tag.getUidString());

  if (tag.hasNdefMessage()) // every tag won't have a message
  {
    NdefMessage message = tag.getNdefMessage();

    // cycle through the records, printing some info from each
    int recordCount = message.getRecordCount();
    if (recordCount > 0) {
      Serial.print(String(recordCount));
    } else {
      Serial.print("no");
    }
    Serial.println(" records found.");

    for (int i = 0; i < recordCount; i++)
    {
      Serial.print("NDEF Record : "); Serial.print(i + 1);
      NdefRecord record = message.getRecord(i);

      int payloadLength = record.getPayloadLength();
      byte payload[payloadLength];
      record.getPayload(payload);
      /*
        // Print the Hex and Printable Characters
        Serial.print(" Payload (HEX): ");
        PrintHexChar(payload, payloadLength);
      */

      // Force the data into a String (might work depending on the content)
      // Real code should use smarter processing
      String payloadAsString = "";
      for (int c = 0; c < payloadLength; c++) {
        payloadAsString += (char)payload[c];
      }
      Serial.print("  Payload (as String): ");
      Serial.print(payloadAsString);

      // end
      Serial.println();
    }
  }
}
