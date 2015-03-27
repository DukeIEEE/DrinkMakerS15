#include <Adafruit_NeoPixel.h>
#ifndef PSTR
//#define PSTR // Make Arduino Due happy
#endif
#define LED_CTRL_PIN 6
#define MUSIC_PLAYING_THRESHOLD 60
#define NUM_LEDS 60
#define LEDS_PER_GROUP 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_CTRL_PIN, NEO_GRB + NEO_KHZ800); //Set up our strip

int colorCounter = 0; // counter for position on the color wheel
int pixelCounter = 0;
int pouringDrink = 0; // a boolean indicating if we are pouring a drink

int NUM_GROUPS = NUM_LEDS / LEDS_PER_GROUP;
unsigned long elapsedTime = 0;
unsigned long drinkStartTime = 0;
int pixelCounters[2] = {0, 35};
uint32_t colors[3] = {strip.Color(255, 0, 0),strip.Color(0, 255, 0),strip.Color(0, 0, 255)};
int iterCounters[2] = {0, 36};
int delayCounter = 0;

// layer settings
uint32_t COLOR0 = strip.Color(150, 255, 255);
uint32_t COLOR1 = strip.Color(85, 255, 255);
uint32_t COLOR2 = strip.Color(40, 170, 255);
uint32_t COLOR3 = strip.Color(0, 140, 255);
uint32_t COLOR4 = strip.Color(0, 100, 255);
uint32_t COLOR5 = strip.Color(0, 40, 255);
uint32_t COLOR6 = strip.Color(0, 0, 255);

const int numColors = 6;
const int numLayers = 12;
char rep_sequence[] = "012345654321";
char sequence[numLayers * numColors];

uint32_t findColor (char c) {
//    char colorToReturn[] = "COLOR";
    if (c == '0') { //white
        return COLOR0;
    }
    else if (c == '1') { //cyan
        return COLOR1;
    }
    else if (c == '2') { //blue
        return COLOR2;
    }
    else if (c == '3') { //blue
        return COLOR3;
    }
    else if (c == '4') {
        return COLOR4;
    }
    else if (c == '5') {
        return COLOR5;
    }
    else { //Duke
        return COLOR6;
    }
}

void createSequence () {
    for (int i = 0; i < numColors * (numLayers - 2); i++) {
        int j = i % (2 * numColors);
        sequence[i] = rep_sequence[j];
    }
}

void showSequence (int wait, int shift) {
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor((i + shift) % NUM_LEDS, findColor(sequence[i]));
    }
    strip.show();
    delay(wait);
}

void bubble (int wait) {
    for (int i = 0; i < numColors; i++) {
        for (int j = 0; j < LEDS_PER_GROUP; j++) {
            strip.setPixelColor(LEDS_PER_GROUP * i + j, findColor(rep_sequence[i % LEDS_PER_GROUP]));
        }
        strip.show();
        delay(wait);
    }
}

void bubbleTrain (int wait) {
    int shiftCount;
    for (int shiftCount = 0; shiftCount < NUM_GROUPS; shiftCount++) {
        strip.clear();
        for (int i = 0; i < NUM_GROUPS; i++) {
            for (int j = 0; j < LEDS_PER_GROUP; j++) {
                strip.setPixelColor(LEDS_PER_GROUP * i + j, findColor(rep_sequence[(shiftCount + i) % (2 * LEDS_PER_GROUP)]));
            }
        }
        strip.show();
        delay(wait);
    }
}

void setup() {
    createSequence();

    // start the led display
    strip.begin();
    delay(100);
    strip.clear();
    Serial.begin(9600);
}

void loop() {
    for (int i = 0; i < NUM_LEDS; i++) {
        bool drinkPoured = (Serial.read() == '0');
        if (!drinkPoured) {
            showSequence(50, i);
        }
        else {
            for (int j = 0; j < 2; j++) {
                bubbleTrain(100);
            }
        }
    }
}

//Theater-style crawling lights with rainbow effect
void spiral (uint8_t wait) {
    // cycle all 256 colors in the wheel
    for (int j = 0; j < 256; j++) {
        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < strip.numPixels(); i++) {
              //turn every third pixel on
                strip.setPixelColor(i + q, Wheel((i + j) % 255));
            }
            strip.show();
            delay(wait);
        }
    }
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t BlueWhiteWheel (byte WheelPos) {
    if (WheelPos < 85) {
        return COLOR6;
    }
    else if (WheelPos < 170) {
        WheelPos -= 85;
        return COLOR0;
    }
    else {
        WheelPos -= 170;
        return (COLOR0 + COLOR6) / 2;
    }
}

// play the default rainbow lights animation
void spiralBlueWhite (int wait) {
    uint16_t i;
    for (i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, BlueWhiteWheel(((i * 256 / strip.numPixels()) + colorCounter) & 255));
    }
    colorCounter++;
    if (colorCounter >= 256) {
        colorCounter = 0;
    }
    delay(wait);
}

// Blue-white gradient
void blueWhiteWipe (int wait, int light0, int lightF) {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        int layer = i / LEDS_PER_GROUP;
        int light = (i + layer) % LEDS_PER_GROUP; //shifting
        if (light <= light0) {
            strip.setPixelColor(i, COLOR6);
        }
        else if (light > lightF) {
            strip.setPixelColor(i, COLOR0);
        }
        else {
            strip.setPixelColor(i, (COLOR0 + COLOR6) / 2);
        }
        strip.show();
        delay(wait);
    }
}

void playBlueWhiteLights (int myDelay) {
    uint16_t i, j;
    for (i = 0; i < LEDS_PER_GROUP; i++) {
        strip.clear();
        for (j = 0; j < NUM_GROUPS; j++) {
            strip.setPixelColor(LEDS_PER_GROUP * j + i, strip.Color(0, 0, 50));
            strip.show();
            delay(myDelay);
            delay(50);
        }
        strip.clear();
        delay(myDelay);
    }
    if (colorCounter >= 255) {
        colorCounter = 0; 
    }
}

// play the default rainbow lights animation
void playRainbowLights(){
    uint16_t i;
    for (i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + colorCounter) & 255));
    }
    colorCounter++;
    if (colorCounter >= 256) {
        colorCounter = 0; 
    }
}

void playSlowRainbow() {
    strip.setPixelColor(pixelCounter, Wheel(colorCounter & 255));
    pixelCounter++;
    if (pixelCounter > 72) {
        pixelCounter = 0; 
        colorCounter++;
    }
    if (colorCounter >= 256) {
        colorCounter = 0; 
    }
}

void playColorSpiral () {
    int i = 0;
    delayCounter++;
    for (i = 0; i < 2; i++) {
        int pixelNumber = pixelCounters[i];
        strip.setPixelColor(pixelNumber, Wheel(iterCounters[i] & 255));
        if (delayCounter > 10) {
            pixelCounters[i] = pixelCounters[i] + 1;
        }
        if (pixelCounters[i] > strip.numPixels()) {
            pixelCounters[i] = 0;
            iterCounters[i] += 36;
            iterCounters[i] %= 255;
        }
    }
    if (delayCounter > 10) {
        delayCounter = 0; 
    }
}

void playColorRainbowChase () {
    uint16_t i, j;
    for (i = 72; i > 0; i -= 8) {
        int colorNumber = 256 - i / 8 * 36;
        if (colorNumber < 0) {
            colorNumber += 256;
        }
        int pixelNumber = i + pixelCounter;
        if (pixelNumber > 72) {
            pixelNumber -= 72; 
        }
        strip.setPixelColor(pixelNumber, Wheel(colorNumber & 255));
    }
    pixelCounter++;
    if (pixelCounter > strip.numPixels()) {
        pixelCounter = 0; 
        colorCounter += 36;
    }
    colorCounter %= 256;
}

uint32_t Wheel (byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if (WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else {
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}
