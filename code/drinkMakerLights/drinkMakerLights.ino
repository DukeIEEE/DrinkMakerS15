#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#ifndef PSTR
//#define PSTR // Make Arduino Due happy
#endif
#define LED_CTRL_PIN 9
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

//const int motorPins[][5] = { // 7 motors per tower
//  {22, 24, 26, 28, 30},
//  {36, 38, 40, 42, 44},
//  {3, 4, 5, 6, 7}
//}; // pump pins
const int motorPins[] = { // 7 motors per tower
  22, 24, 26, 28, 30
}; // pump pins

const long motorTimes[] = {
  17, 30, 30, 30, 17
}; //seconds needed to dispense one shot

char inputData; // Data input from bluetooth data
int activatedPiece[2] = {
  0, 0
}; // 2D array, index 0 for the towers (values 0 through 2) and index 1 for the pumps (values 0 through 4) or the valve (value 5)
int isSelectingTower = 0;
int selectedTower = 0;
int isTypingRecipe = 0; // A boolean indicating whether a recipe is being typed
int drinkIndex = 0; // the index of the current drink we are entering
int buttonState = 0;  // variable for reading the pushbutton status
int oldButtonState = 0; // variable for holding the previous button state
int spectrumValue[7]; // to hold audio spectrum values
int drinkAmounts[] = {
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

const int numColors = 6;
const int numLayers = 12;
char rep_sequence[] = "012345654321";
char sequence[numLayers * numColors];

void setup () {
  createSequence();

  // start the led display
  strip.begin();
  delay(100);

  strip.clear();

  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.println("finished setup");
  Serial1.println("finished setup");
}

void loop () {
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
    delay(5); // change this to modify animation speed
    listenForBluetoothAndAct(); // receive bluetooth messages
    pourDrink();
  }
  strip.show();
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
    strip.show();
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
      cancelAll();
    }

    // if we're typing a recipe, add the current value to the recipe
    if (isTypingRecipe) { // isTypingRecipe && !isSelectingTower
      // if we're not putting in a comma, add the # to the next digit of the current drink
      if (inputData == '0' || inputData == '1' || inputData == '2' || inputData == '3' || inputData == '4' ||
          inputData == '5' || inputData == '6' || inputData == '7' || inputData == '8' || inputData == '9') {
        Serial.println("number");
        drinkAmounts[drinkIndex] *= 10;
        drinkAmounts[drinkIndex] += inputData;
      }
      // if we did get a comma, move to the next number and check to see if our drink should be prepared
      if (inputData == ',') {
        Serial.println("COMMA");
        drinkIndex++;
        // if we've entered all the available drink values, make the drink
        if (drinkIndex >= (sizeof(drinkAmounts) / sizeof(int))) {
          drinkIndex = 0;
          isTypingRecipe = 0;
          isPouringDrink = 1;
          drinkStartTime = millis();
          elapsedTime = 0;
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
      // type an 'f' to flush the system
      if (inputData == 'f') {
        Serial.println("Flush");
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
    }
  }
}

// pour a drink according to the hundredths of a shot that were fed in
void pourDrink () {
  if (isPouringDrink) {
    Serial.print("Making drink in Tower");
    Serial.println(selectedTower);
    elapsedTime = millis() - drinkStartTime;
    int isPumpStillOn = 0;
    // go through the pumps. If we've poured our amounts, turn off the pump
    for (int i = 0; i < sizeof(motorPins) / sizeof(int); i++) {
      Serial.println((long) drinkAmounts[i] * motorTimes[i] * 10);
      Serial.println(elapsedTime);
      if (((long) drinkAmounts[i] * motorTimes[i] * 10) <= elapsedTime) {
        digitalWrite(motorPins[i], LOW);
        Serial.println("LOW");
      }
      else {
        digitalWrite(motorPins[i], HIGH);
        Serial.println("HIGH");
        isPumpStillOn = 1;
      }
      Serial.println();
    }

    // If we're done making the drink, finish the process
    // Should this boolean be reversed?
    if (!isPumpStillOn) {
      cancelAll();
    }
  }
}

// cancel a drink recipe
void cancelAll () {
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
void clearDrinkAmounts () {
  for (int i = 0; i < (sizeof(drinkAmounts) / sizeof(int)); i++) {
    drinkAmounts[i] = 0;
  }
}

// check flush button press and enable/disable all pumps accordingly
void checkAndActOnFlushState () {
  // read the flush button and set pumps accordingly
  buttonState = digitalRead(buttonPin);
  if (!buttonState) { // if the button is pressed
    setAllPumps(HIGH); // turn on the pumps
  }
  else if (buttonState && (buttonState != oldButtonState)) { // if the button is not pressed and was previously pressed
    setAllPumps(LOW); // turn off the pumps
  }
  oldButtonState = buttonState;
}

// set all pumps to a given value
void setAllPumps (int state) {
  uint16_t i;
  for (i = 0; i < (sizeof(motorPins) / sizeof(int)); i++) {
    digitalWrite(motorPins[i], state);
  }
}

