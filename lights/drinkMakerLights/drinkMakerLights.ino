
#include <Adafruit_NeoPixel.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
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
void setup()
{
 // start the led display
 strip.begin();
}

void loop()
{  
  playBlueWhiteLights(); // show the rainbow light animation
  delay(100);

}

void playBlueWhiteLights(){
  uint16_t i, j;
  for(i = 0; i < LEDS_PER_GROUP; i++) {
    strip.clear();
    for(j = 0; j < NUM_GROUPS; j++){
      strip.setPixelColor(LEDS_PER_GROUP*j + i, strip.Color(0, 0, 50));
      strip.show();
      delay(50);
    }
    strip.clear();
  }
  if(colorCounter>=255){
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
