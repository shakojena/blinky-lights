#include "FastLED.h"

// How many leds in your strip(s)?
#define NUM_LEDS 100

// Below is boilerplate code from some of the FastLED examples - see there for more info
#define DATA_PIN 7

// Define the array of leds
CRGB leds[NUM_LEDS];
int sensorInputPin = 2;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int pirValue = 0;                    // variable for reading the pin status
int alert_red = 0;                    // counter to show red leds
int alert_green = 0;                  // counter to show green leds - will override red.


void setup() {
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
}

void loop() {
  pirValue = digitalRead(sensorInputPin);  // read input value
  if (pirValue == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;

    }

    for (int i = 0; i < NUM_LEDS; i++) {
      if (i % 11 == alert_green)
      {
        leds[i] = CRGB::Green;
      }
      else if (i % 3 == alert_red)
      {
        leds[i] = CRGB::Red;
      }
      else
      {
        leds[i] = CRGB::Black;
      }
    }
    //      for (int i = alert_red; i < NUM_LEDS; i+=3) {
    //        leds[i].CRGB::Red;
    //      }
    alert_red++;
    alert_green++;
    alert_red %= 3;
    alert_green %= 11;
    FastLED.show();
    delay(50);
    return;
  }
  if (pirState == HIGH) {
    // we have just turned of
    Serial.println("Motion ended!");
    // We only want to print on the output change, not state
    pirState = LOW;
  }
  //else
  cylon();

}

void setEvery(int step) {
  // TODO make alternating more generic
}

void marquee() {
  // TODO
  int step = NUM_LEDS / 10;
  int half_step = step / 2;
  for (int dot = 0; dot < NUM_LEDS / 10; dot++) {
    leds[dot] = CRGB::Red;
    //leds[dot+ half_step] = CRGB:Green;
    FastLED.show();
    delay(30);
    leds[dot] = CRGB::Black;
    //leds[dot+ half_step] = CRGB:Black;
    FastLED.show();
  }
}




void fadeall_cylon() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].r = scale8(leds[i].r, 250);
    leds[i].g = scale8(leds[i].g, 250);
    leds[i].b = scale8(qadd8(leds[i].b, 25), 150);
  }
}

void cylon() {
  static uint8_t hue = 0;

  // First slide the led in one direction
  for (int i = 0; i < NUM_LEDS; i++) {

    if ( hue == 150 ) { // Skip the blue range, since we're defaulting them to blue. 
      hue = 190;
    }
    
    // Set the i'th led to the current hue
    leds[i] = CHSV(hue++, 255, 255);
    
    // Show the leds
    FastLED.show();
    
    // now that we've shown the leds, fade all of them a little bit;
    fadeall_cylon();
    // Wait a little bit before we loop around and do it again
    delay(10);
  }

  // Now go in the other direction.
  for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
    if ( hue == 150 ) { //Skip the blue range, since we're defaulting them to blue. 
      hue = 190;
    }
    // Set the i'th led to the current hue
    leds[i] = CHSV(hue++, 255, 255);
    // Show the leds
    FastLED.show();
    
    // Wait longer in the reverse direction
    fadeall_cylon();
    delay(30);
//    fadeall_cylon();
//    delay(10);
//    fadeall_cylon();
//    delay(10);
  }
}
