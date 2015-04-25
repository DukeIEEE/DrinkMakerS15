#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#ifndef PSTR
//#define PSTR // make Arduino Due happy
#endif
#define LED_CTRL_PIN 9
#define NUM_LEDS 60
#define LEDS_PER_GROUP 6

int NUM_GROUPS = NUM_LEDS / LEDS_PER_GROUP;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, LED_CTRL_PIN, NEO_GRB + NEO_KHZ800); // set up our strip

const int buttonPin = 9; // the number of the pushbutton pin
const int analogPin = A5; // multiplexer analog read pin
const int strobePin = 2; // multiplexer strobe pin
const int resetPin = 3; // multiplexer reset pin

char inputData = 'x'; // data input from bluetooth data

// Booleans
int isPouringDrink[] = { // can be true for any number of towers
  0, 0, 0
};
int isAnyPumpStillOn[] = { // can be true for any number of towers
  0, 0, 0
};
int isSelectingTower = 0; // mutually exclusive for all towers
int isTypingRecipe = 0; // mutually exclusive for all towers

int activatedPiece[2] = { // 2D array, index 0 for the towers (values 0 through 2, left to right) and index 1 for the pumps (values 0 through 4, clockwise from the space)
  0, 0
};

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
  {22, 24, 26, 28, 30}, // tower 0
  {36, 38, 40, 42, 44}, // tower 1
  {3, 4, 5, 6, 7}       // tower 2
};
const long motorTimes[][5] = { // seconds needed to dispense one shot; motors ordered clockwise from space... calibrate?
  {17, 30, 30, 30, 17}, // tower 0
  {17, 30, 30, 30, 17}, // tower 1
  {17, 30, 30, 30, 17}  // tower 2
};

int selectedTower = 0;  // the index of the tower selected from left to right
int drinkIndex = 0;     // the index of the current drink we are entering

// color values for LED strip
uint32_t WHITE = strip.Color(150, 255, 255);
uint32_t COLOR1 = strip.Color(85, 255, 255);
uint32_t COLOR2 = strip.Color(40, 170, 255);
uint32_t COLOR3 = strip.Color(0, 140, 255);
uint32_t COLOR4 = strip.Color(0, 100, 255);
uint32_t COLOR5 = strip.Color(0, 40, 255);
uint32_t BLUE = strip.Color(0, 0, 255);

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
  Serial3.begin(9600);
  Serial.println("finished setup");
  Serial3.println("finished setup");
}

void loop () {
  for (int i = 0; i < NUM_LEDS; i++) {
    bool drinkPoured = Serial.read() == '0' | Serial3.read() == '0';
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
    delay(100); // change this to modify animation speed
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
  if (Serial3.available()) {
    inputData = Serial3.read();
    Serial.print("inputdata = ");
    Serial.println(inputData);

    // type an 'x' to set all booleans to false
    if (inputData == 'x') {
      Serial.println("All booleans reset to 0");
      cancelAllActionsForSelectedTower();
    }

    // if we're typing a recipe, add the current value to the recipe
    if (isTypingRecipe) { // isTypingRecipe && !isSelectingTower
      // if we're not putting in a comma, add the # to the next digit of the current drink
      if (inputData == '0' || inputData == '1' || inputData == '2' || inputData == '3' || inputData == '4' ||
          inputData == '5' || inputData == '6' || inputData == '7' || inputData == '8' || inputData == '9') {
        Serial.println("number");
        drinkAmounts[selectedTower][drinkIndex] *= 10;
        drinkAmounts[selectedTower][drinkIndex] += inputData;
      }
      // if we did get a comma, move to the next number and check to see if our drink should be prepared
      if (inputData == ',') {
        Serial.println("COMMA");
        drinkIndex++;
        // if we've entered all the available drink values, make the drink
        if (drinkIndex >= (sizeof(drinkAmounts) / sizeof(int))) {
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
      // type an 'f' to flush the system
      if (inputData == 'f') {
        setAllPumps(HIGH);
        Serial.println("Flush all pumps on selected tower");
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
  if (isPouringDrink[selectedTower]) {
    Serial.print("Making drink in Tower ");
    Serial.println(selectedTower);
    elapsedTime[selectedTower] = millis() - drinkStartTime[selectedTower];
    for (int i = 0; i < sizeof(isAnyPumpStillOn); i++) { // reset all pumps for all towers to LOW
      isAnyPumpStillOn[i] = 0;
    }
    // go through the pumps. If we've poured our amounts, turn off the pump
    for (int j = 0; j < sizeof(motorPins) / sizeof(int); j++) {
      for (int i = 0; i < sizeof(motorPins[selectedTower]) / sizeof(int); i++) {
        //        for (int n = 0; n < 15 / sizeof(int); n++) { // may need to use hardcode
        if ((j + 1) * (i + 1) == 15) {
          Serial3.println("don't need to hardcode");
        }
        if ((j + 1) * (i + 1) > 15) {
          Serial3.println("smh...");
        }

        Serial.println((long) drinkAmounts[selectedTower][i] * motorTimes[selectedTower][i] * 10);
        Serial.println(elapsedTime[selectedTower]);
        if (((long) drinkAmounts[selectedTower][i] * motorTimes[selectedTower][i] * 10) <= elapsedTime[selectedTower]) {
          digitalWrite(motorPins[selectedTower][i], LOW);
          Serial.println("LOW");
        }
        else {
          digitalWrite(motorPins[selectedTower][i], HIGH);
          Serial.println("HIGH");
          isAnyPumpStillOn[selectedTower] = 1;
        }
      }
    }

    // If we're done making the drink, finish the process
    // Should this boolean be reversed?
    if (!isAnyPumpStillOn[selectedTower]) {
      cancelAllActionsForSelectedTower();
    }
  }
}

// cancel a drink recipe
void cancelAllActionsForSelectedTower () {
  Serial.print("Done with drink. The current selected tower is Tower ");
  Serial.println(selectedTower);
  clearDrinkAmountsForSelectedTower();
  setAllPumps(LOW);
  isPouringDrink[selectedTower] = 0;
  isSelectingTower = 0;
  isTypingRecipe = 0;
  drinkIndex = 0;
}

// set all drink amounts to 0
void clearDrinkAmountsForSelectedTower () {
  for (int i = 0; i < (sizeof(drinkAmounts) / sizeof(int)); i++) {
    drinkAmounts[selectedTower][i] = 0;
  }
}

// set all pumps to a given value
void setAllPumps (int state) {
  for (uint16_t i = 0; i < (sizeof(motorPins) / sizeof(int)); i++) {
    digitalWrite(motorPins[selectedTower][i], state);
  }
}

