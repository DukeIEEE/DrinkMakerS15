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
int pixelCounters[2] = {0,35};
uint32_t colors[3] = {strip.Color(255,0,0),strip.Color(0,255,0),strip.Color(0,0,255)};
int iterCounters[2] = {0,36};
int delayCounter=0;

// layer settings
uint32_t BLUE = strip.Color(0, 0, 255);
uint32_t WHITE = strip.Color(150, 255, 255);
uint32_t COLOR1 = strip.Color(85,255,255);
uint32_t COLOR2 = strip.Color(40,170,255);
uint32_t COLOR3 = strip.Color(0,140,255);
uint32_t COLOR4 = strip.Color(0,100,255);
uint32_t COLOR5 = strip.Color(0,40,255);

//int index[12][6];

const int numColors = 6;
const int numLayers = 12;
char rep_sequence[] = "012345654321";
char sequence[numLayers*numColors];

uint32_t findColor(char c) {
  if(c=='0') { //white
    return WHITE;
  } else if(c=='1') { //cyan
    return COLOR1;
  } else if(c=='2') { //blue
    return COLOR2;
  } else if(c=='3') { //blue
    return COLOR3;
  } else if(c=='4') {
    return COLOR4;
  } else if(c=='5') {
    return COLOR5;
  } else { //Duke
    return BLUE;
  }
}

void createSequence() {
  for(int i=0; i<numColors*(numLayers-2); i++) {
    int j = i%(2*numColors);
//    for (int j = 0; j < numColors * 2 - 2; j++) {
//      sequence[i] = rep_sequence[j%numColors];
//    }
    sequence[i] = rep_sequence[j];
  }
}

void setup() {
  createSequence();
  
  // start the led display
  strip.begin();
  delay(100);
//  showSequence(50);

  strip.clear();
}

void loop() {
//  for(int i=1000; i>0; i=i-5) {
//    playBlueWhiteLights(50); // show the rainbow light animation
//    delay(i);
//  }
//  for(int i=50; i>0; i=i-5) {
//    strip.clear();
//    blueWhiteWipe(i, 2,3);
//    delay(100);
//    strip.show();
//  }
//  blueWhiteWipe(50, 2,3);
//  spiralBlueWhite(50);

  for(int i=0; i<NUM_LEDS; i++) {
    showSequence(50,i);
  }
}

void showSequence(int wait, int shift) {
//  uint16_t i;
  for(int i=0; i<NUM_LEDS; i++) {
    strip.setPixelColor((i+shift)%NUM_LEDS, findColor(sequence[i]));
  }
  strip.show();
  delay(wait);
}

//
//uint32_t colorMix(uint32_t a, uint32_t b) {
//  uint8_t r = a.
//}

//Theatre-style crawling lights with rainbow effect
void spiral(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+1) {
          strip.setPixelColor(i+q, Wheel((i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
//        for (int i=0; i < strip.numPixels(); i=i+3) {
//          strip.setPixelColor(i+q, 0);        //turn every third pixel off
//        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t BlueWhiteWheel(byte WheelPos) {
//  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return BLUE;
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return WHITE;
  } else {
   WheelPos -= 170;
   return (BLUE+WHITE)/2;
  }
}

// play the default rainbow lights animation
void spiralBlueWhite(int wait){
  uint16_t i;
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, BlueWhiteWheel(((i * 256 / strip.numPixels()) + colorCounter) & 255));
  }
  colorCounter++;
  if(colorCounter>=256){
    colorCounter = 0; 
  }
  delay(wait);
}

// Blue-white gradient
void blueWhiteWipe(int wait, int light0, int lightf) {
  for(uint16_t i=0; i<NUM_LEDS; i++) {
      int layer = i/LEDS_PER_GROUP;
      int light = (i+layer)%LEDS_PER_GROUP; //shifting
      if(light<=light0) {
        strip.setPixelColor(i, BLUE);
      } else if(light>lightf) {
        strip.setPixelColor(i, WHITE);
      } else {
        strip.setPixelColor(i, (BLUE+WHITE)/2);
      }
      strip.show();
      delay(wait);
  }
}

void playBlueWhiteLights(int myDelay) {
  uint16_t i, j;
  for(i = 0; i < LEDS_PER_GROUP; i++) {
    strip.clear();
    for(j = 0; j < NUM_GROUPS; j++){
      strip.setPixelColor(LEDS_PER_GROUP*j + i, strip.Color(0, 0, 50));
      strip.show();
      delay(myDelay);
      delay(50);
    }
    strip.clear();
    delay(myDelay);
  }
  if(colorCounter>=255) {
    colorCounter = 0; 
  }
}

// play the default rainbow lights animation
void playRainbowLights(){
  uint16_t i;
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + colorCounter) & 255));
    }
    colorCounter++;
    if(colorCounter>=256){
      colorCounter = 0; 
    }
}
void playSlowRainbow(){
 strip.setPixelColor(pixelCounter, Wheel(colorCounter & 255));
 pixelCounter++;
 if(pixelCounter>72){
  pixelCounter = 0; 
  colorCounter++;
 }
 if(colorCounter>=256){
      colorCounter = 0; 
    }
}
void playColorSpiral(){
  int i =0;
  delayCounter++;
  for(i=0;i<2;i++)
  {
    int pixelNumber  = pixelCounters[i];
    strip.setPixelColor(pixelNumber,Wheel(iterCounters[i]&255));
    if(delayCounter>10){
    pixelCounters[i] = pixelCounters[i]+1;
    }
    if(pixelCounters[i]>strip.numPixels()){
     pixelCounters[i] = 0;
      iterCounters[i]+=36;
      if(iterCounters[i]>255)
      {
       iterCounters[i]-=255; 
      }
      
    }
  }
  if(delayCounter>10){
   delayCounter = 0; 
  }
}
void playColorRainbowChase(){
  uint16_t i, j;
    for(i=72; i>0; i-=8){
      int colorNumber = 256-i/8*36;
      if(colorNumber<0)
      {
        colorNumber+=256;
      }
      int pixelNumber = i+pixelCounter;
      if(pixelNumber>72){
       pixelNumber-=72; 
      }
      strip.setPixelColor(pixelNumber, Wheel(colorNumber & 255));
    }
    
    pixelCounter++;
    if(pixelCounter>strip.numPixels()){
     pixelCounter = 0; 
     colorCounter+=36;
    }
    
    if(colorCounter>256){
       colorCounter-=256; 
    }
 
 
 //for(i=72;i>0;i-=5)
 //{
  // int tempColor = colorCounter - (i);
 //  if(tempColor<0){
 //   tempColor +=255; 
 //  }
 //  int pixel = i+pixelCounter;
 //  if(pixel>72)
 //  {
//    pixel-=72; 
//   }
 //   strip.setPixelColor(pixel, Wheel(tempColor & 255));
// }
// pixelCounter++;
// if(pixelCounter>72)
// {
 // pixelCounter = 0; 
// }
 // colorCounter++;
 //if(colorCounter>=256){
 //     colorCounter = 0; 
 //   }
}
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

