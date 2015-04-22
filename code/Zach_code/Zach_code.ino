
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>// import the serial library
#ifndef PSTR
#define PSTR // Make Arduino Due happy
#endif
#define LED_CTRL_PIN 6
#define MUSIC_PLAYING_THRESHOLD 60

#define rxPin 19
#define txPin 18

const int buttonPin = 9;     // the number of the pushbutton pin
const int analogPin = A5; // multiplexer analog read pin
const int strobePin = 2; // multiplexer strobe pin
const int resetPin = 3; // multiplexer reset pin
const int motorPins[] = {A0, A1, A2, A3, A4}; // pump pins
const long motorTimes[] = {17, 30, 30, 30, 17}; //seconds needed to dispense one shot

// set up a new serial port
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(72, LED_CTRL_PIN, NEO_GRB + NEO_KHZ800); //Set up our strip

int inputData; // Data input from bluetooth data
int isTypingRecipie = 0; // A boolaen indicating whether a recipie is being types
int drinkIndex = 0; // the index of the current drink we are entering
int colorCounter = 0; // counter for position on the color wheel
int pixelCounter = 0; // counter used for position in the pixel string
int buttonState = 0;  // variable for reading the pushbutton status
int oldButtonState = 0; // variable for holding the previous button state
int spectrumValue[7]; // to hold audio spectrum values
int drinkAmounts[5] = {0, 0, 0, 0, 0}; // hundredths of a shot for the current drink
int musicMode = 0; // a boolean indicating if we are playing music
int pouringDrink = 0; // a boolean indicating if we are pouring a drink
unsigned long elapsedTime = 0;
unsigned long drinkStartTime = 0;
int pixelCounters[2] = {0, 35};
uint32_t colors[3] = {strip.Color(255, 0, 0), strip.Color(0, 255, 0), strip.Color(0, 0, 255)};
int iterCounters[2] = {0, 36};
int delayCounter = 0;
void setup()
{
  mySerial.begin(9600); // setup bluetooth comms
  Serial.begin(9600);// setup serial comms for debugging
  Serial.println("finished init");
  // mySerial.println("Drink Maker Connected... Waiting for input");
  // set up motor pins
  uint16_t i;
  for (i = 0; i < sizeof(motorPins) / sizeof(int); i++) {
    pinMode(motorPins[i], OUTPUT);
  }
  // set up pins for audio spectrum reading
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  // set up pin for flush reading
  pinMode(buttonPin, INPUT_PULLUP);

  // initialize the filter IC
  analogReference(DEFAULT);
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  // start the led display
  strip.begin();
  strip.show();
}

void loop()
{
  checkAndActOnFlushState(); // check if we need to flush out the motors
  playRainbowLights(); // show the rainbow light animation
  checkForAndActOnMusicPlaying();  // make lights pretty if we're playing music
  listenForBluetoothAndAct();   // receive bluetooth messages
  pourDrink();
  strip.show();
  delay(10);

}
// pour a drink according to the hundredths of a shot that were fed in
void pourDrink() {

  if (pouringDrink) {
    Serial.println("Making drink");
    elapsedTime = millis() - drinkStartTime;
    int pumpsStillOn = 0;
    // go through the pumps. If we've poured our amounts, turn off the pump
    for (int i = 0; i < sizeof(motorPins) / sizeof(int); i++) {
      Serial.println((long)drinkAmounts[i]*motorTimes[i] * 10);
      Serial.println(elapsedTime);
      if (((long)drinkAmounts[i]*motorTimes[i] * 10) <= elapsedTime)
        digitalWrite(motorPins[i], LOW);
      else {
        digitalWrite(motorPins[i], HIGH);
        pumpsStillOn = 1;
      }
      Serial.println();
    }

    // If we're done making the drink, finish the process
    if (!pumpsStillOn)
      cancelDrinkMaking();
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

// check to see if music is playing. if it is, turn off lights
// selectively in the tower to reflect the music levels
void checkForAndActOnMusicPlaying() {
  if (musicMode) {
    uint16_t i;
    updateAudioFrequencies(); // get our frequencies
    for (i = 0; i < 6; i++)
      showAudioLevel(i, spectrumValue[i]);
  }
}

// get the various frequency amplitudes from the MSGEQ7
void updateAudioFrequencies() {
  uint16_t i;
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(50); // to allow the output to settle
    spectrumValue[i] = analogRead(analogPin);
    digitalWrite(strobePin, HIGH);
  }
}
// turn off lights selectively in the tower to show audio bands if music is playing
void showAudioLevel(uint8_t rowNum, uint16_t audioLevel) {
  uint16_t i;
  uint8_t lights = (audioLevel) / 50 + 1;
  for (i = 10; i > lights - 1; i--) {
    strip.setPixelColor((i) * 6 + rowNum, 0);
  }
}

// check flush button press and enable/ disable all pumps accordingly
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
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void listenForBluetoothAndAct() {
  // if we have a bluetooth message
  if (Serial.available() > 0 || mySerial.available() > 0) {
    Serial.println("got stuff");
    inputData = mySerial.read();
    Serial.println(inputData);
    // if we're typing a recipie, add the current value to the recipie
    if (isTypingRecipie == 1) {
      // if we're not putting in a comma, add the # to the next digit of the current drink
      if (inputData != ',') {
        drinkAmounts[drinkIndex] *= 10;
        drinkAmounts[drinkIndex] += inputData;
      }
      // if we did get a comma, move to the next number and check to see if our drink should be prepared
      else {
        drinkIndex++;
        // if we've entered all the available drink values, make the drink
        if (drinkIndex >= sizeof(drinkAmounts) / sizeof(int)) {
          drinkIndex = 0;
          isTypingRecipie = 0;
          pouringDrink = 1;
          drinkStartTime = millis();
          elapsedTime = 0;
        }
      }
    }
    // type a '\' to start typing a drink
    if (inputData == '\\' && !pouringDrink)
      isTypingRecipie = 1;
    // type an 'x' to stop making drink
    if (inputData == 'x')
      cancelDrinkMaking();
    // type a 'm' to change light modes
    if (inputData == 'm') {
      musicMode = !musicMode;
    }
  }
}

// cancel a drink recipie
void cancelDrinkMaking() {
  Serial.println("Done with drink");
  clearDrinkAmounts();
  setAllPumps(LOW);
  isTypingRecipie = 0;
  pouringDrink = 0;
  drinkIndex = 0;
}

// set all drink amounts to 0
void clearDrinkAmounts() {
  for (int i = 0; i < sizeof(drinkAmounts) / sizeof(int); i++) {
    drinkAmounts[i] = 0;
  }
}

