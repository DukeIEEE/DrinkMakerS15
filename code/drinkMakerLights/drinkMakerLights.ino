#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
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
int isPouringDrink = 0; // a boolean indicating if we are pouring a drink

int NUM_GROUPS = NUM_LEDS / LEDS_PER_GROUP;
unsigned long elapsedTime = 0;
unsigned long drinkStartTime = 0;
int pixelCounters[2] = {
  0, 35
};
uint32_t colors[3] = {
  strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255)
};
int iterCounters[2] = {
  0, 36
};
int delayCounter = 0;


const int buttonPin = 9;     // the number of the pushbutton pin
const int analogPin = A5; // multiplexer analog read pin
const int strobePin = 2; // multiplexer strobe pin
const int resetPin = 3; // multiplexer reset pin
const int motorPins[] = {
  A0, A1, A2, A3, A4
}; // pump pins
const long motorTimes[] = {
  17, 30, 30, 30, 17
}; //seconds needed to dispense one shot

char inputData; // Data input from bluetooth data
int activatedPiece[2] = {0, 0}; // 2D array, index 0 for the towers (values 0 through 2) and index 1 for the pumps (values 0 through 4) or the valve (value 5)
int isSelectingTower = 0;
int selectedTower = 0;
int isTypingRecipe = 0; // A boolaen indicating whether a recipe is being types
int drinkIndex = 0; // the index of the current drink we are entering
int buttonState = 0;  // variable for reading the pushbutton status
int oldButtonState = 0; // variable for holding the previous button state
int spectrumValue[7]; // to hold audio spectrum values
int drinkAmounts[5] = {
  0, 0, 0, 0, 0
}; // hundredths of a shot for the current drink


// layer settings
uint32_t BLUE = strip.Color(0, 0, 255);
uint32_t WHITE = strip.Color(150, 255, 255);
uint32_t COLOR1 = strip.Color(85, 255, 255);
uint32_t COLOR2 = strip.Color(40, 170, 255);
uint32_t COLOR3 = strip.Color(0, 140, 255);
uint32_t COLOR4 = strip.Color(0, 100, 255);
uint32_t COLOR5 = strip.Color(0, 40, 255);

//int index[12][6];

const int numColors = 6;
const int numLayers = 12;
char rep_sequence[] = "012345654321";
char sequence[numLayers * numColors];

//char bubbleSeq[2*numColors] = rep_sequence;

uint32_t findColor(char c) {
  if (c == '0') { //white
    return WHITE;
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
    return BLUE;
  }
}

void createSequence() {
  for (int i = 0; i < numColors * (numLayers - 2); i++) {
    int j = i % (2 * numColors);
    //    for (int j = 0; j < numColors * 2 - 2; j++) {
    //      sequence[i] = rep_sequence[j%numColors];
    //    }
    sequence[i] = rep_sequence[j];
  }
}

void showSequence(int wait, int shift) {
  //  uint16_t i;
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor((i + shift) % NUM_LEDS, findColor(sequence[i]));
    //    Serial.println("debug_sequence");
    delay(10);
    listenForBluetoothAndAct();   // receive bluetooth messages
  }
  strip.show();
  delay(wait);
}

void bubble(int wait) {
  for (int i = 0; i < numColors; i++) {
    for (int j = 0; j < LEDS_PER_GROUP; j++) {
      strip.setPixelColor(LEDS_PER_GROUP * i + j, findColor(rep_sequence[i % LEDS_PER_GROUP]));
    }
    strip.show();
    delay(wait);
  }
}

void bubbleTrain(int wait) {
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
  //  showSequence(50);

  strip.clear();

  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("finished setup");
  Serial1.println("finished setup");
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

  for (int i = 0; i < NUM_LEDS; i++) {
    bool drinkPoured = Serial.read() == '0' | Serial1.read() == '0';
    if (drinkPoured) {
      Serial.println('0');
    }
    if (!drinkPoured) {
      showSequence(50, i);
    }
    else {
      for (int j = 0; j < 2; j++) {
        bubbleTrain(100);
      }
    }
  }

  //bubble(500);
  //  bubbleTrain(200);
}

//
//uint32_t colorMix(uint32_t a, uint32_t b) {
//  uint8_t r = a.
//}

//Theatre-style crawling lights with rainbow effect
void spiral(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 1) {
        strip.setPixelColor(i + q, Wheel((i + j) % 255)); //turn every third pixel on
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
  if (WheelPos < 85) {
    return BLUE;
  }
  else if (WheelPos < 170) {
    WheelPos -= 85;
    return WHITE;
  }
  else {
    WheelPos -= 170;
    return (BLUE + WHITE) / 2;
  }
}

// play the default rainbow lights animation
void spiralBlueWhite(int wait) {
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
void blueWhiteWipe(int wait, int light0, int lightf) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    int layer = i / LEDS_PER_GROUP;
    int light = (i + layer) % LEDS_PER_GROUP; //shifting
    if (light <= light0) {
      strip.setPixelColor(i, BLUE);
    }
    else if (light > lightf) {
      strip.setPixelColor(i, WHITE);
    }
    else {
      strip.setPixelColor(i, (BLUE + WHITE) / 2);
    }
    strip.show();
    delay(wait);
  }
}

void playBlueWhiteLights(int myDelay) {
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
void playRainbowLights() {
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
void playColorSpiral() {
  int i = 0;
  delayCounter++;
  for (i = 0; i < 2; i++)
  {
    int pixelNumber  = pixelCounters[i];
    strip.setPixelColor(pixelNumber, Wheel(iterCounters[i] & 255));
    if (delayCounter > 10) {
      pixelCounters[i] = pixelCounters[i] + 1;
    }
    if (pixelCounters[i] > strip.numPixels()) {
      pixelCounters[i] = 0;
      iterCounters[i] += 36;
      if (iterCounters[i] > 255)
      {
        iterCounters[i] -= 255;
      }
    }
  }
  if (delayCounter > 10) {
    delayCounter = 0;
  }
}
void playColorRainbowChase() {
  uint16_t i, j;
  for (i = 72; i > 0; i -= 8) {
    int colorNumber = 256 - i / 8 * 36;
    if (colorNumber < 0)
    {
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

  if (colorCounter > 256) {
    colorCounter -= 256;
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

// Bluetooth
void listenForBluetoothAndAct() {
  
  // if we have a bluetooth connection
  if (Serial1.available()) {
    inputData = Serial1.read();
    Serial.print("inputdata = ");
    Serial.println(inputData);

    // type an 'x' to set all booleans to false
    if (inputData == 'x') {
      Serial.println("All booleans reset to 0");
      cancelAll();
    }

    // type an 'f' to flush the system
    if (inputData == 'f') {
      checkAndActOnFlushState();
    }

    // type 't' to open tower selection
    if (inputData == 't') {
      isSelectingTower = 1;
      Serial.println("Tower selection opened");
    }

    // type a 'p' to start typing a drink recipe
    if (inputData == 'p' && !isPouringDrink) {
      Serial.println("Start typing a drink recipe");
      isTypingRecipe = 1;
    }

    // if we're typing a recipe, add the current value to the recipe
    if (isTypingRecipe) {
      // if we're not putting in a comma, add the # to the next digit of the current drink
      if (inputData == '0' || inputData == '1' || inputData == '2' || inputData == '3' || inputData == '4' ||
          inputData == '5' || inputData == '6' || inputData == '7' || inputData == '8' || inputData == '9') {
        Serial.println("number");
        drinkAmounts[drinkIndex] *= 10;
        drinkAmounts[drinkIndex] += inputData;
      }
      // if we did get a comma, move to the next number and check to see if our drink should be prepared
      else {
        Serial.println("COMMA");
        drinkIndex++;
        // if we've entered all the available drink values, make the drink
        if (drinkIndex >= sizeof(drinkAmounts) / sizeof(int)) {
          drinkIndex = 0;
          isTypingRecipe = 0;
          isPouringDrink = 1;
          drinkStartTime = millis();
          elapsedTime = 0;
        }
      }
    }
    // if we're not typing a recipe, we can turn on isSelectingTower
    else if (isSelectingTower) { // !isTypingRecipe
      if (inputData == '1' || inputData == '2' || inputData == '3') {
        selectedTower = inputData - 49;
        isSelectingTower = 0;
        Serial.print("Selected tower: Tower ");
        Serial.println(selectedTower);
      }
      else { // incorrect input
        Serial.println("Please type 1, 2, or 3 to select tower");
      }
    }
  }
}

// cancel a drink recipe
void cancelAll() {
  Serial.print("Done with drink. The current selected tower is Tower ");
  Serial.println(selectedTower);
  clearDrinkAmounts();
  setAllPumps(LOW);
  isTypingRecipe = 0;
  isPouringDrink = 0;
  isSelectingTower = 0;
  drinkIndex = 0;
}

// set all drink amounts to 0
void clearDrinkAmounts() {
  for (int i = 0; i < sizeof(drinkAmounts) / sizeof(int); i++) {
    drinkAmounts[i] = 0;
  }
}

// pour a drink according to the hundredths of a shot that were fed in
void pourDrink() {
  if (isPouringDrink) {
    Serial.println("Making drink");
    elapsedTime = millis() - drinkStartTime;
    int isPumpStillOn = 0;
    // go through the pumps. If we've poured our amounts, turn off the pump
    for (int i = 0; i < sizeof(motorPins) / sizeof(int); i++) {
      Serial.println((long)drinkAmounts[i]*motorTimes[i] * 10);
      Serial.println(elapsedTime);
      if (((long)drinkAmounts[i]*motorTimes[i] * 10) <= elapsedTime)
        digitalWrite(motorPins[i], LOW);
      else {
        digitalWrite(motorPins[i], HIGH);
        isPumpStillOn = 1;
      }
      Serial.println();
    }

    // If we're done making the drink, finish the process
    if (!isPumpStillOn)
      cancelAll();
  }
}

// check flush button press and enable/disable all pumps accordingly
void checkAndActOnFlushState() {
  // read the flush button and set pumps accordingly
  buttonState = digitalRead(buttonPin);
  if (!buttonState) // if the button is pressed
    setAllPumps(HIGH); // turn on the pumps
  else if (buttonState && buttonState != oldButtonState) // if the button is not pressed and was previously pressed
    setAllPumps(LOW); // turn off the pumps
  oldButtonState = buttonState;
}

// set all pumps to a given value
void setAllPumps(int state) {
  uint16_t i;
  for (i = 0; i < sizeof(motorPins) / sizeof(int); i++) {
    digitalWrite(motorPins[i], state);
  }
}

