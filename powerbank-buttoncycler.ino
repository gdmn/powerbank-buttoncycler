// ====================================
//                ATtiny
//               25/45/85
//              +--------+
//            --+ o  Vcc +------------
//  LED - PB3 --+        +-- PB2
//        PB4 --+        +-- PB1 - BTN
//  ------------+ GND    +-- PB0
//              +--------+
// ====================================
// Requires headers for AVR defines and ISR function
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN    PB3    // Digital IO pin connected to the NeoPixels.

#define PIXEL_COUNT 16

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

volatile int showType = 0;
volatile int currentShowType = showType;
volatile long previousMillis = 0;

#define INTERRUPT_PIN PCINT1  // This is PB1 per the schematic
#define INT_PIN PB1           // Interrupt pin of choice: PB1 (same as PCINT1) - Pin 6
#define PCINT_VECTOR PCINT0_vect      // This step is not necessary - it's a naming thing for clarit

void setup() {
  cli();                            // Disable interrupts during setup
  PCMSK |= (1 << INTERRUPT_PIN);    // Enable interrupt handler (ISR) for our chosen interrupt pin (PCINT1/PB1/pin 6)
  GIMSK |= (1 << PCIE);             // Enable PCINT interrupt in the general interrupt mask
  pinMode(INT_PIN, INPUT_PULLUP);   // Set our interrupt pin as input with a pullup to keep it stable
  sei();                            //last line of setup - enable interrupts after setup
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(128);
  showType = -1;
  currentShowType = 0;

  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.setPixelColor(1, strip.Color(0, 255, 0));
  strip.setPixelColor(2, strip.Color(0, 0, 255));
  strip.show();
}

// This is the interrupt handler called when there is any change on the INT_PIN
// ISR is defined in the headers - the ATtiny85 only has one handler
ISR(PCINT_VECTOR) {
  unsigned long currentMillis = millis();
   
  if(currentMillis - previousMillis > 200) {
    if (digitalRead(INT_PIN) == LOW) {
      showType++;
      if (showType > 10) {
        showType=0;
      }
    }
  }
  previousMillis = currentMillis;
}

void loop() {  
  startShow(showType);
}

void startShow(int i) {
  if (i < 0 || i > 10) {
    return;
  }
  uint8_t wait = 50;
  uint8_t waitRainbow = 120;
  uint8_t waitChase = 250;
  currentShowType = i;
  switch(i){
    case 0: colorWipe(strip.Color(30, 30, 30), wait);    
            break;
    case 1: colorWipe(strip.Color(255, 255, 255), wait);   
            break;
    case 2: rainbow(waitRainbow);
            break;
    case 3: rainbowCycle(waitRainbow);
            break;
    case 4: colorWipe(strip.Color(255, 0, 0), wait);  // Red
            break;
    case 5: colorWipe(strip.Color(0, 255, 0), wait);  // Green
            break;
    case 6: colorWipe(strip.Color(0, 0, 255), wait);  // Blue
            break;
    case 7: theaterChase(strip.Color(127, 127, 127), waitChase); // White
            break;
    case 8: theaterChase(strip.Color(127,   0,   0), waitChase); // Red
            break;
    case 9: theaterChase(strip.Color(  0,   0, 127), waitChase); // Blue
            break;
    case 10: theaterChaseRainbow(waitChase);
            break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
    if (showType != currentShowType) return;
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
    if (showType != currentShowType) return;
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
    if (showType != currentShowType) return;
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);
      if (showType != currentShowType) return;

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);
      if (showType != currentShowType) return;

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
