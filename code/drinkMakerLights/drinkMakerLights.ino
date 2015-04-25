#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h> //delete?
#ifndef PSTR
//#define PSTR // make Arduino Due happy
#endif
#define LED_CTRL_PIN 9
#define NUM_LEDS 180
#define LEDS_PER_GROUP 6

int NUM_GROUPS = NUM_LEDS / LEDS_PER_GROUP;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_CTRL_PIN, NEO_GRB + NEO_KHZ800); // set up our strip -- number LEDs?

const int buttonPin = 9; // the number of the pushbutton pin -- delete?
const int analogPin = A5; // multiplexer analog read pin -- delete?
const int strobePin = 2; // multiplexer strobe pin -- delete?
const int resetPin = 3; // multiplexer reset pin -- delete?

char inputData; // data input from bluetooth data

// Booleans
int isPouringDrink[] = { // can be true for any number of towers
  0, 0, 0
};
int isAnyPumpStillOn[] = { // can be true for any number of towers
  0, 0, 0
};
int isSelectingTower = 0; // mutually exclusive for all towers
int isTypingRecipe = 0; // mutually exclusive for all towers
int isStripDisplayingRainbowLights = 1;

int activatedPiece[2] = { // 2D array, index 0 for the towers (values 0 through 2, left to right) and index 1 for the pumps (values 0 through 4, clockwise from the space)
  0, 0
};

// Arrays
unsigned long drinkStartTime[] = { // start time of pouring drink; towers ordered left to right
  0, 0, 0
};
unsigned long elapsedTime[] = { // elapsed time of pouring drink; towers ordered left to right
  0, 0, 0
};
int drinkAmounts[][5] = { // hundredths of a shot for the current drink; towers ordered left to right
  {0, 0, 0, 0, 0}, // tower 0
  {0, 0, 0, 0, 0}, // tower 1
  {0, 0, 0, 0, 0}  // tower 2
};
const int motorPins[][5] = { // pump pins; motors ordered clockwise from space
  {30, 22, 26, 24, 28}, // tower 0
  {36, 40, 44, 38, 42}, // tower 1
  {4, 7, 5, 6, 3}       // tower 2
};
const long motorTimes[][5] = { // seconds needed to dispense one shot; motors ordered clockwise from space... calibrate?
  {15, 30, 30, 30, 15}, // tower 0
  {15, 30, 30, 30, 15}, // tower 1
  {15, 30, 30, 30, 15}  // tower 2
};

// Initial states
int selectedTower = 0;  // the index of the tower selected from left to right
int drinkIndex = 0;     // the index of the current drink we are entering

// Color constants for LED strip
uint32_t WHITE = strip.Color(150, 255, 255);
uint32_t COLOR1 = strip.Color(85, 255, 255);
uint32_t COLOR2 = strip.Color(40, 170, 255);
uint32_t COLOR3 = strip.Color(0, 140, 255);
uint32_t COLOR4 = strip.Color(0, 100, 255);
uint32_t COLOR5 = strip.Color(0, 40, 255);
uint32_t BLUE = strip.Color(0, 0, 255);

// Constants for blue-white animations
const int numColors = 6;
const int numLayers = 30;
char rep_sequence[] = "012345654321";
char sequence[numLayers * numColors];

// Constants for full-color animations
int colorCounter = 0; // counter for position on the color wheel
int pixelCounter = 0;
int pixelCounters[2] = {0, 35};
uint32_t colors[3] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255)};
int iterCounters[2] = {0, 36};
int delayCounter = 0;

void setup () {
  createSequence();

  // start the led display
  strip.begin();
  delay(100);

//  strip.clear();
//  strip.show();

  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("finished setup");
  Serial1.println("finished setup");
}

void loop () {
    playRainbowLights();
    strip.show();
    delay(10);
    listenForBluetoothAndAct(); // receive bluetooth messages
    pourDrink();
//  if (isStripDisplayingRainbowLights) {
//    delay(100); // change this to modify animation speed
//    listenForBluetoothAndAct(); // receive bluetooth messages
//    pourDrink();
//  }
//  else {
//    for (int i = 0; i < NUM_LEDS; i++) {
//      bool drinkPoured = Serial.read() == '0' | Serial1.read() == '0';
//      if (drinkPoured) {
//        Serial.println('0');
//      }
//      if (!drinkPoured) {
//        showSequence(50, i);
//      }
//      else {
//        for (int j = 0; j < 2; j++) {
//          bubbleTrain(100);
//        }
//      }
//    }
//  }
}

void createSequence () {
  for (int i = 0; i < numColors * (numLayers - 2); i++) {
    int j = i % (2 * numColors);
    sequence[i] = rep_sequence[j];
  }
}

void showSequence (int wait, int lightPositionOffset) { // refactor when the machine works?
  for (int i = 0; i < NUM_LEDS; i++) { // main loop
    strip.setPixelColor((i + lightPositionOffset) % NUM_LEDS, findColor(sequence[i]));
    delay(100); // change this to modify animation speed
    listenForBluetoothAndAct(); // receive bluetooth messages
    pourDrink();
  }
  //  strip.show();
  delay(wait);
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
    //    strip.show();
    delay(wait);
  }
}

uint32_t findColor (char c) {
  if (c == '0') { // White
    return WHITE;
  }
  else if (c == '1') {
    return COLOR1;
  }
  else if (c == '2') {
    return COLOR2;
  }
  else if (c == '3') {
    return COLOR3;
  }
  else if (c == '4') {
    return COLOR4;
  }
  else if (c == '5') {
    return COLOR5;
  }
  else { // Duke Blue
    return BLUE;
  }
}

// Bluetooth
void listenForBluetoothAndAct () {

  // if we have a bluetooth connection
  if (Serial1.available()) {
    inputData = Serial1.read();
    Serial.print("inputdata = ");
    Serial.println(inputData);

    // type an 'x' to set all booleans to false
    if (inputData == 'x') {
      Serial.println("All booleans reset to 0");
      cancelAllActionsForSelectedTower(selectedTower);
    }

    // if we're typing a recipe, add the current value to the recipe
    if (isTypingRecipe) { // isTypingRecipe && !isSelectingTower
      // if we're not putting in a comma, add the # to the next digit of the current drink
      if (inputData == '0' || inputData == '1' || inputData == '2' || inputData == '3' || inputData == '4' ||
          inputData == '5' || inputData == '6' || inputData == '7' || inputData == '8' || inputData == '9') {
        Serial.println("number");
        drinkAmounts[selectedTower][drinkIndex] *= 10;
        drinkAmounts[selectedTower][drinkIndex] += inputData - 48;
        Serial.println("inputData: " );
        Serial.println(inputData - 48);
        Serial1.println("inputData: " );
        Serial1.println(inputData - 48);
      }
      // if we did get a comma, move to the next number and check to see if our drink should be prepared
      if (inputData == ',') {
        // print drink recipe being poured
        for (int i = 0; i < 5; i++) {
          Serial.print("Tower #");
          Serial.print(selectedTower);
          Serial.print(", Pump #");
          Serial.println(i);
          Serial.print("Drink Amount: ");
          Serial.println(drinkAmounts[selectedTower][i]);
          
          Serial1.print("Tower #");
          Serial1.print(selectedTower);
          Serial1.print(", Pump #");
          Serial1.println(i);
          Serial1.print("Drink Amount: ");
          Serial1.println(drinkAmounts[selectedTower][i]);
        }
        Serial.println("COMMA");
        drinkIndex++;
        // if we've entered all the available drink values, make the drink
        if (drinkIndex >= (sizeof(drinkAmounts[0]) / sizeof(int))) { // -- added "[0]"
          drinkIndex = 0;
          isTypingRecipe = 0;
          isPouringDrink[selectedTower] = 1;
          drinkStartTime[selectedTower] = millis();
          elapsedTime[selectedTower] = 0;
        }
      }
    }
    else if (isSelectingTower) { // !isTypingRecipe && isSelectingTower
      if (inputData == '0' || inputData == '1' || inputData == '2') {
        selectedTower = inputData - 48;
        isSelectingTower = 0;
        Serial.print("Selected tower: Tower ");
        Serial.println(selectedTower);
      }
      else { // incorrect input
        Serial.println("Please type 0, 1, or 2 to select tower");
      }
    }
    // if we're not typing a recipe, we can turn on isTypingRecipe or isSelectingTower
    else { // !isTypingRecipe && !isSelectingTower
      // type 't' to open tower selection
      if (inputData == 't') {
        isSelectingTower = 1;
        Serial.println("Tower selection opened");
      }
      // type a 'p' to start typing a drink recipe
      if (inputData == 'p' && !isPouringDrink[selectedTower]) {
        Serial.println("Start typing a drink recipe");
        isTypingRecipe = 1;
      }
      // type an 'f' to flush the system
      if (inputData == 'f' && !isPouringDrink[selectedTower]) {
        setAllPumps(HIGH, selectedTower);
        Serial.println("Flush all pumps on selected tower");
      }
    }
  }
}

// pour a drink according to the hundredths of a shot that were fed in
void pourDrink () {
  for (int tower = 0; tower < 3; tower++) {
    if (isPouringDrink[tower]) {
      elapsedTime[tower] = millis() - drinkStartTime[tower];
      isAnyPumpStillOn[tower] = 0;

      for (int i = 0; i < sizeof(motorPins[tower]) / sizeof(int); i++) {
        if (((long) drinkAmounts[tower][i] * motorTimes[tower][i] * 10) <= elapsedTime[tower]) {
          digitalWrite(motorPins[tower][i], LOW);
        }
        else {
          digitalWrite(motorPins[tower][i], HIGH);
          isAnyPumpStillOn[tower] = 1;
        }
      }
      // If we're done making the drink, finish the process
      // Should this boolean be reversed?
      if (!isAnyPumpStillOn[tower]) {
        cancelAllActionsForSelectedTower(tower);
      }
    }
  }
}

// cancel a drink recipe
void cancelAllActionsForSelectedTower (int towerNum) {
  Serial.print("Done with drink. The current selected tower is Tower ");
  Serial.println(selectedTower);
  clearDrinkAmountsForSelectedTower(towerNum);
  setAllPumps(LOW, towerNum);
  isPouringDrink[towerNum] = 0;
  isSelectingTower = 0;
  isTypingRecipe = 0;
  drinkIndex = 0;
}

// set all drink amounts to 0
void clearDrinkAmountsForSelectedTower (int towerNum) {
  for (int i = 0; i < (sizeof(drinkAmounts[0]) / sizeof(int)); i++) { // added "[0]"
    drinkAmounts[towerNum][i] = 0;
  }
}

// set all pumps to a given value
void setAllPumps (int state, int towerNum) {
  for (uint16_t i = 0; i < (sizeof(motorPins[0]) / sizeof(int)); i++) { // -- added "[0]"
    digitalWrite(motorPins[towerNum][i], state);
  }
}

//------------------------------------------------------------------------//
//                        FULL-COLOR FUNCTIONALITY
//------------------------------------------------------------------------//

//Theater-style crawling lights with rainbow effect
void spiral (uint8_t wait) {
  // cycle all 256 colors in the wheel
  for (int j = 0; j < 256; j++) {
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i++) {
        //turn every third pixel on
        strip.setPixelColor(i + q, Wheel((i + j) % 255));
      }
      //            strip.show();
      delay(wait);
    }
  }
}

// play the default rainbow lights animation
void playRainbowLights () {
  uint16_t i;
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + colorCounter) & 255) / 1.5); // change brightness
  }
  colorCounter++;
  if (colorCounter >= 256) {
    colorCounter = 0;
  }
}

void playSlowRainbow () {
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

