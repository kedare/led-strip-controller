// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

#include "application.h"
#include "neopixel/neopixel.h"

SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT 60
#define PIXEL_TYPE WS2812B

// How much to divide the light intensity, 100 is maximum power, 0 is off
#define DEFAULT_POWER 80

#define DEFAULT_MODE "off"
#define DEFAULT_WAIT 1000

#define DEFAULT_COLOR 0xFF40FF

/* ======================= Prototype Defs =========================== */

void colorAll(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Init default mode
String mode = DEFAULT_MODE;
uint32_t wait = DEFAULT_WAIT;
uint32_t power = DEFAULT_POWER;

// TODO DRY
uint16_t colorRgb1[3];
long colorHex1 = DEFAULT_COLOR;
String colorString1;
uint16_t colorRgb2[3];
long colorHex2 = DEFAULT_COLOR;
String colorString2;


void setup() {
    Particle.variable("mode", mode);
    Particle.variable("wait", wait);
    Particle.variable("power", power);
    Particle.variable("color1", colorString1);
    Particle.variable("color2", colorString2);
    Particle.function("setMode", setMode);
    Particle.function("setWait", setWait);
    Particle.function("setPower", setPower);
    Particle.function("setColor1", setColor1);
    Particle.function("setColor2", setColor2);
    setColor1(String(DEFAULT_COLOR, HEX));
    setColor2(String(DEFAULT_COLOR, HEX));
    strip.begin();
    strip.show();
}

float getIntermediate(float a, float b, float maxStep, float step) {
  return a + (b - a) * ((step/100*maxStep)/100);
}

int setMode(String newMode) {
    mode = newMode;
    return 0;
}

int setWait(String newWait) {
    wait = newWait.toInt();
    return 0;
}

int setPower(String newPower) {
    power = newPower.toInt();
    return 0;
}


// TODO DRY
int setColor1(String newColor) {
  colorHex1 = (long) strtol(newColor, NULL, 16);
  int r = colorHex1 >> 16;
  int g = colorHex1 >> 8 & 0xFF;
  int b = colorHex1 & 0xFF;
  colorRgb1[0] = r;
  colorRgb1[1] = g;
  colorRgb1[2] = b;
  colorString1 = "#" + String(colorHex1, HEX);
  return 1;
}

int setColor2(String newColor) {
  colorHex2 = (long) strtol(newColor, NULL, 16);
  int r = colorHex2 >> 16;
  int g = colorHex2 >> 8 & 0xFF;
  int b = colorHex2 & 0xFF;
  colorRgb2[0] = r;
  colorRgb2[1] = g;
  colorRgb2[2] = b;
  colorString2 = "#" + String(colorHex2, HEX);
  return 1;
}

void loop() {
    if (mode == "colorWipe") {
        colorWipe(strip.Color(colorRgb1[0]*(power*0.01), colorRgb1[1]*(power*0.01), colorRgb1[2]*(power*0.01)));
    }
    else if(mode == "steadyColor") {
        steadyColor(strip.Color(colorRgb1[0]*(power*0.01), colorRgb1[1]*(power*0.01), colorRgb1[2]*(power*0.01)));
    }
    else if (mode == "fadeCycle") {
        fadeCycle(100, colorRgb1[0], colorRgb1[1], colorRgb1[2], colorRgb2[0], colorRgb2[1], colorRgb2[2]);
    }
    else if (mode == "gradient") {
        gradient(colorRgb1[0], colorRgb1[1], colorRgb1[2], colorRgb2[0], colorRgb2[1], colorRgb2[2]);
    }
    else if (mode == "rainbow") {
        rainbow();
    }
    else if (mode == "rainbowCycle") {
        rainbowCycle();
    }
    else if (mode == "fullColorCycle") {
        fullColorCycle();
    }
    else if (mode == "randomDots") {
        randomDots();
    }
    else if (mode == "off") {
        turnedOff();
    }
    else if (mode == "frozen") {
        delay(1000);
    }
    else {
        turnedOff();
    }
  // Some example procedures showing how to display to the pixels:
  // Do not run more than one of these at a time, or the b/g tasks
  // will be blocked.
  //--------------------------------------------------------------
  //strip.setPixelColor(0, strip.Color(255, 0, 255));
  //strip.show();

  //colorWipe(strip.Color(255, 0, 0), 50); // Red

  //colorWipe(strip.Color(0, 255, 0), 50); // Green

  //colorWipe(strip.Color(0, 0, 255), 50); // Blue

  //rainbow(20);

  //rainbowCycle(0);

  //fullColorCycle(2000);

  //fadeCycle(100, 10, 255, 0, 0, 0, 255, 255);

  //randomDot(5);

  //colorAll(strip.Color(0, 255, 255), 50); // Magenta
}

void steadyColor(uint32_t c) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
    delay(1000);
}

void turnedOff() {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0,0,0));
    }
    strip.show();
    delay(1000);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      if(mode != "colorWipe") { break; }
      delay(wait);
  }
}

void rainbow() {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    if(mode != "rainbow") { break; }
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowCycle() {
  uint16_t i, j;
  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    if(mode != "rainbowCycle") { break; }
    delay(wait);
  }
}

// Rotating on all the colors on the whole strip (Uniform colors)
void fullColorCycle() {
  uint16_t i, j;
  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((1 * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    if(mode != "fullColorCycle") { break; }
    delay(wait);
  }
}

void randomDots() {
  uint16_t i, currentDot, maxR, maxG, maxB;
  float j;
  currentDot = random(1,strip.numPixels());
  maxR = random(1,255);
  maxG = random(1,255);
  maxB = random(1,255);

  // Turn off everything
  for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, strip.Color(0,0,0));
  }

  strip.show();
  for(j=0; j<101; j++) { // 1 cycle of all colors on wheel
    strip.setPixelColor(currentDot, strip.Color(maxR*(j/100)*(power*0.01) ,maxG*(j/100)*(power*0.01) ,maxB*(j/100)*(power*0.01) ));
    strip.show();
    if(mode != "randomDots") { break; }
    delay(wait);
  }
  for(j=100; j>0; j--) { // 1 cycle of all colors on wheel
    strip.setPixelColor(currentDot, strip.Color(maxR*(j/100)*(power*0.01) ,maxG*(j/100)*(power*0.01) ,maxB*(j/100)*(power*0.01)));
    strip.show();
    if(mode != "randomDots") { break; }
    delay(wait);
  }
  strip.setPixelColor(currentDot, strip.Color(0,0,0));
}

void fadeCycle(uint16_t steps, uint16_t r1, uint16_t g1, uint16_t b1, uint16_t r2, uint16_t g2, uint16_t b2) {
    uint16_t i, j, rn, gn, bn;
    for(j=1; j<steps; j++) {
        rn = getIntermediate(r1, r2, steps, j);
        gn = getIntermediate(g1, g2, steps, j);
        bn = getIntermediate(b1, b2, steps, j);
        for(i=1; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(rn*(power*0.01), gn*(power*0.01), bn*(power*0.01)));
        }
        strip.show();
        delay(wait);
    }

    for(j=1; j<steps; j++) {
        rn = getIntermediate(r2, r1, steps, j);
        gn = getIntermediate(g2, g1, steps, j);
        bn = getIntermediate(b2, b1, steps, j);
        for(i=1; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, strip.Color(rn*(power*0.01), gn*(power*0.01), bn*(power*0.01)));
        }
        strip.show();
        delay(wait);
    }
}

// Working weird
void gradient(uint16_t r1, uint16_t g1, uint16_t b1, uint16_t r2, uint16_t g2, uint16_t b2) {
  uint16_t i, pixels, rn, gn, bn;
  pixels = strip.numPixels();

  for(i=1; i<pixels; i++) {
      rn = getIntermediate(r1, r2, pixels, i);
      gn = getIntermediate(g1, g2, pixels, i);
      bn = getIntermediate(b1, b2, pixels, i);
      strip.setPixelColor(i, strip.Color(rn*(power*0.01), gn*(power*0.01), bn*(power*0.01)));
      strip.show();
  }
  delay(1000);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color((WheelPos * 3)*(power*0.01), (255 - WheelPos * 3)*(power*0.01), 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color((255 - WheelPos * 3)*(power*0.01), 0, (WheelPos * 3)*(power*0.01));
  } else {
   WheelPos -= 170;
   return strip.Color(0, (WheelPos * 3)*(power*0.01), (255 - WheelPos * 3)*(power*0.01));
  }
}
