/*
   Written, maintained by : Sébastien Albert
   V 0.0.1
*/

#include "Osc.h"
#include "NFC.h"

// NEOPIXEL SECTION -------------------------------------------------------
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define LED_PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, LED_PIN, NEO_GRB + NEO_KHZ800);

int bright = 255;
// ------------------------------------------------------------------------

#define CTRL_LED 13

void setup() {
  // Init the serial
  Serial.begin(115200);
  Serial.println("Hello World !");
  
  pinMode(CTRL_LED, OUTPUT);
  // Init the Shield
  NFCInit();
  // Init connection
  OscInit();

  // Led
  strip.begin();

}

void loop() {

  // Check connection and talk about it
  wifiCheck();

  char currentTag = nfcGetNewTag();

  // --------------------------------------------------
  // IN = First, I had no tag and I put it in
  // --------------------------------------------------
  if (lastTag != currentTag) {
    // -------------------------------------------
    Serial.print("Brand New Tag : [");
    Serial.print(String(currentTag));
    Serial.println("]");

    //sendTag(currentTag, IN_BONUS, 0);
    sendIt("/lastTag", currentTag);
    lastTag = currentTag;
  }

  // --------------------------------------------------
  // nothing, print a dot
  // --------------------------------------------------
  else {
    /*
      Serial.print("Current Tag : [");
      Serial.print(String(currentTag));
      Serial.print("]");
      Serial.print(" Last Tag : [");
      Serial.print(String(lastTag));
      Serial.println("]");
    */
    Serial.print(".");
  }

  // ----------------------------------------------------------------
  /*
  bright += 2;
    if(bright >= 255){
    bright = 0;
  }
  Serial.print("Bright is : ");
  Serial.println(bright);
  */
  
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(bright, bright, bright)); // Moderately bright green color.
  }
  strip.show(); // This sends the updated pixel color to the hardware.

}
