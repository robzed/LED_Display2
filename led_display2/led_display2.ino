//#include <FastLED.h>
#include <Adafruit_NeoPixel.h>
#define NUM_ROWS 24
#define NUM_COLUMNS 24
#define NUM_LEDS (NUM_ROWS*NUM_COLUMNS)
#define DATA_PIN 6

//CRGB leds[NUM_LEDS];

Adafruit_NeoPixel pixels(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 500


void setup() {
  pixels.begin();
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  
  // http://fastled.io/docs/3.1/class_c_fast_l_e_d.html#a730ba7d967e882b4b893689cf333b2eb
  //FastLED.setBrightness(64);
  //Serial.begin(115200);
  //Serial.println(F("\nLED+2\n"));
  pinMode(LED_BUILTIN, OUTPUT);
}


#if 1
void loop() {

   for(int col = 0; col < NUM_COLUMNS; col++) { 
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        pixels.setPixelColor(col+row, pixels.Color(150, 0, 0));
      }
      pixels.show();
      // clear this led for the next time around the loop
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        pixels.setPixelColor(col+row, pixels.Color(0, 0, 0));
      }
      delay(30);
  }
   for(int col = 0; col < NUM_COLUMNS; col++) { 
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        pixels.setPixelColor(col+row, pixels.Color(0, 150, 0));
      }
      pixels.show();
      // clear this led for the next time around the loop
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        pixels.setPixelColor(col+row, pixels.Color(0, 0, 0));
      }
      delay(30);
  }
   for(int col = 0; col < NUM_COLUMNS; col++) { 
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        pixels.setPixelColor(col+row, pixels.Color(0, 0, 150));
      }
      pixels.show();
      // clear this led for the next time around the loop
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        pixels.setPixelColor(col+row, pixels.Color(0, 0, 0));
      }
      delay(30);
  }
}
#endif

#if 0
// the loop function runs over and over again forever
void loop() {
  pixels.clear();

  for(int i=0; i < NUM_LEDS; i++) {

    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(DELAYVAL);
  }

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
#endif 

#if 0
void loop() {
    for(int dot = 0; dot < NUM_LEDS; dot++) { 
        leds[dot] = CRGB::White;
        FastLED.show();
        // clear this led for the next time around the loop
        leds[dot] = CRGB::Black;
        delay(30);
    }
    for(int dot = 0; dot < NUM_LEDS; dot++) { 
        leds[dot] = CRGB::Red;
        FastLED.show();
        // clear this led for the next time around the loop
        leds[dot] = CRGB::Black;
        delay(30);
    }
    for(int dot = 0; dot < NUM_LEDS; dot++) { 
        leds[dot] = CRGB::Green;
        FastLED.show();
        // clear this led for the next time around the loop
        leds[dot] = CRGB::Black;
        delay(30);
    }
    for(int dot = 0; dot < NUM_LEDS; dot++) { 
        leds[dot] = CRGB::Blue;
        FastLED.show();
        // clear this led for the next time around the loop
        leds[dot] = CRGB::Black;
        delay(30);
    }
}
#endif

#if 0
void loop() {

   for(int col = 0; col < NUM_COLUMNS; col++) { 
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        leds[col+row] = CRGB::Blue;
      }
      FastLED.show();
      // clear this led for the next time around the loop
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        leds[col+row] = CRGB::Black;
      }
      delay(30);
  }
   for(int col = 0; col < NUM_COLUMNS; col++) { 
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        leds[col+row] = CRGB::Red;
      }
      FastLED.show();
      // clear this led for the next time around the loop
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        leds[col+row] = CRGB::Black;
      }
      delay(30);
  }
   for(int col = 0; col < NUM_COLUMNS; col++) { 
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        leds[col+row] = CRGB::Green;
      }
      FastLED.show();
      // clear this led for the next time around the loop
      for(int row = 0; row < NUM_LEDS; row += NUM_COLUMNS) { 
        leds[col+row] = CRGB::Black;
      }
      delay(30);
  }
}
#endif
