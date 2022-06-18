// Libraries and constants

#include <FastLED.h>
#define NUM_LEDS 226 // set to total number of LEDs
#define DATA_PIN 4 // if Wemos, data pin 4 is labelled D2 
#define LED_TYPE WS2812B // set to type of LED strip
#define COLOUR_ORDER GRB // set according to type of LED strip
#define BRIGHTNESS  222 // adjust as required

#define IR_SMALLD_NEC        //1st: define which protocol to use and then,
#include <IRsmallDecoder.h>  //2nd: include the library;
IRsmallDecoder irDecoder(2); //3rd: create one decoder object with the correct digital pin;
irSmallD_t irData;           //4th: declare one decoder data structure;
const byte pressUp = 24;
const byte pressDown = 82;
const byte pressRight = 90;
const byte pressLeft = 8;
const byte pressOK = 28;
byte keypress = 0;

//  Define LED array CRGB leds[NUM_LEDS];
CRGBArray<NUM_LEDS> leds;

/* Digit map:
          4                3                   2             1

          A
       -------          -------             -------       -------
      |       |        |       |           |       |     |       |
    F |       | B      |       |    * 112  |       |     |       |
      |   G   | 170    |       | 114       |       | 56  |       | 0
       -------          -------             -------       -------
      |       |        |       |           |       |     |       |
    E |       | C      |       |    * 113  |       |     |       |
      |       |        |       |           |       |     |       |
       -------          -------             -------       -------
          D
*/

// Digit starts
const byte pps = 8;  // number of Pixels Per Segment
byte digitStart = 0;
const byte digitStart1 = 0; // starting pixel of digit 1
const byte digitStart2 = 56; // starting pixel of digit 2
const byte digitStart3 = 114; // starting pixel of digit 3
const byte digitStart4 = 170; // starting pixel of digit 4

// All digits and dots (see displayDigit function for segments)

CRGBSet segAll(  leds(0,  NUM_LEDS)  );
CRGBSet segDots(  leds(112,  113)  );

// Colours (red by default; white uses too much current)
byte r = 255;
byte g = 0;
byte b = 0;

// Timekeeping

int timeStart; // starting time in minutes (no seconds).
int timeEnd; // end time in minutes (no seconds).
int countStart; // calculated starting time in count integers. Calculated during program mode (blue).
int countEnd; // caculated end time in count integers. Calculated during program mode (blue).
int timeCount = 0;  // keeps track of what number to display. Calculated during program mode (blue).

unsigned long previousMillis = 0; // used instead of delay
unsigned long currentMillis = 0;
long intervalMillis = 1000; // interval between timeCount ticks. Should normally be 1000 (1 second) but can be adjusted for calibration.

byte seconds; // calculated from timeCount in getTime function.
byte secondsOnes; // calculated from seconds in getTime function.
byte secondsTens; // calculated from seconds in getTime function.
byte minutes; // calculated from timeCount in getTime function.
byte minutesOnes; // calculated from minutes in getTime function.
byte minutesTens; // calculated from minutes in getTime function.

// Clock modes and time options

byte clockMode = 0; // Modes: 0 = select time; 1 = pause; 2 = run; 3 = finished.
byte timeOption = 0; // Options: 0 = no change; 1 = 0-45; 2 = 45-90; 3 = 0-15; 4 = 15-30.

//-------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);  // Allows serial monitor output (check baud rate)
  Serial.println("Autobots, transform!");
  delay(3000); // 3 second delay for recovery
  Serial.println("Autobots, roll out!");

  // FastLED start
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOUR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();

  // Light it up
  clockMode = 0; // launch to program mode
  Serial.println("Clock Mode 0; launching to program mode.");
  getTime();

}

//-------------------------------------------------------------------------------------

void loop() {

  // Run each loop

  // All program modes

  if (keypress = ! 0) {
    keypress = 0;
  }
  if (irDecoder.dataAvailable(irData) && irData.keyHeld == 0) {
    if (irData.cmd == pressLeft || irData.cmd == pressRight || irData.cmd == pressUp || irData.cmd == pressDown || irData.cmd == pressOK) {
      Serial.print("Press ");
      keypress = irData.cmd; //set keypress to applicable press for this loop
      if (keypress == pressLeft) {
        Serial.println("left");
      }
      else if (keypress == pressRight) {
        Serial.println("right");
      }
      else if (keypress == pressUp) {
        Serial.println("up");
      }
      else if (keypress == pressDown) {
        Serial.println("down");
      }
      else if (keypress == pressOK) {
        Serial.println("OK");
      }
    }
  }

  if (keypress == pressLeft) { // Always exit to program mode on left press
    clockMode = 0;
    Serial.println("Clock Mode 0; program mode");
  }

  if (keypress == pressRight || keypress == pressOK) {
    currentMillis = previousMillis = millis(); // set currentMillis and previousMillis to milliseconds since power on on keypress
    if (clockMode == 2) { // If running, pause
      clockMode = 1;
      Serial.println("Clock Mode 1; pause mode");
    }
    else {
      clockMode = 2; // If in any other mode, start running
      Serial.println("Clock Mode 2; run mode");
    }
    showTime();
  }


  //----------------------------------------------

  // Program mode (blue)

  if (clockMode == 0) {

    if (r != 0 || g != 0 || b != 255) {
      r = 0;  // If not already blue, set colour to blue.
      g = 0;
      b = 255;
      timeOption = 0; // Retain previous time in case of accidental press.
      showTime();
    }

    // Set clock using up and down press
    if (keypress == pressDown) {
      if (timeOption > 1) {
        timeOption--;
      } else {
        timeOption = 1;
      }
      setTime();
    }

    if (keypress == pressUp) {
      if (timeOption < 4) {
        timeOption++;
      } else {
        timeOption = 4;
      }
      setTime();
    }
  }


  //----------------------------------------------

  // Pause and run mode (red)

  if (clockMode == 1 || clockMode == 2) {
    currentMillis = millis(); // set currentMillis to milliseconds since power on
    if (r != 255 || g != 0 || b != 0) {
      r = 255;  // If not already red, set colour to red
      g = 0;
      b = 0;
      getTime();
    }

    // Button presses during run mode

    if (keypress == pressUp) { // If up button pressed during run mode, add 1 minute.
      timeCount = timeCount + 60;
      getTime();
    }

    if (keypress == pressDown) { // If down button pressed during run mode, subtract 1 minute.
      if (timeCount < 60) {
        timeCount = 0;
      }
      else {
        timeCount = timeCount - 60;
      }
      getTime();
    }
  }

  if (clockMode == 2) {
    if (timeCount < countEnd) { // If time to run
      if (currentMillis - previousMillis >= intervalMillis) {
        previousMillis = currentMillis;
        timeCount++;
        getTime();
      }
    }
    else { // If timeCount == timeEnd
      clockMode = 3;
    }
  }

  //----------------------------------------------

  // Stop mode (yellow)

  if (clockMode == 3) {
    if (r != 240 || g != 120 || b != 0) {
      r = 240;  // If not already yellow, set colour to yellow
      g = 120;
      b = 0;
      getTime();
    }
  }

}

//-------------------------------------------------------------------------------------

void getTime() {
  seconds = timeCount % 60;
  secondsOnes = seconds % 10;
  secondsTens = seconds / 10;
  minutes = timeCount / 60;
  minutesOnes = minutes % 10;
  minutesTens = minutes / 10;
  showTime();
}

//-------------------------------------------------------------------------------------

void setTime() {
  if (timeOption == 1) { // 0 - 45
    timeStart = 0; timeEnd = 45;
    minutesTens = 0; minutesOnes = 0; secondsTens = 4; secondsOnes = 5; // display start and end times
  }
  else if (timeOption == 2) { // 45 - 90
    timeStart = 45; timeEnd = 90;
    minutesTens = 4; minutesOnes = 5; secondsTens = 9; secondsOnes = 0; // display start and end times
  }
  else if (timeOption == 3) { // 0 - 15
    timeStart = 0; timeEnd = 15;
    minutesTens = 0; minutesOnes = 0; secondsTens = 1; secondsOnes = 5; // display start and end times
  }
  else if (timeOption == 4) { // 15 - 30
    timeStart = 15; timeEnd = 30;
    minutesTens = 1; minutesOnes = 5; secondsTens = 3; secondsOnes = 0; // display start and end times
  }

  if (timeOption != 0) { // i.e. Don't reset count if time option zero
    countStart = timeStart * 60; // calculated starting time in count integers
    countEnd = timeEnd * 60; // caculated end time in count integers
    timeCount = 0 + countStart;  // keeps track of what number to display
  }
  showTime();
}
//-------------------------------------------------------------------------------------

void displayDigit(int x) {
  CRGBSet segB(  leds((pps * 0) + digitStart,  (pps - 1 + (pps * 0)) + digitStart  ));
  CRGBSet segA(  leds((pps * 1) + digitStart,  (pps - 1 + (pps * 1)) + digitStart  ));
  CRGBSet segF(  leds((pps * 2) + digitStart,  (pps - 1 + (pps * 2)) + digitStart  ));
  CRGBSet segE(  leds((pps * 3) + digitStart,  (pps - 1 + (pps * 3)) + digitStart  ));
  CRGBSet segD(  leds((pps * 4) + digitStart,  (pps - 1 + (pps * 4)) + digitStart  ));
  CRGBSet segC(  leds((pps * 5) + digitStart,  (pps - 1 + (pps * 5)) + digitStart  ));
  CRGBSet segG(  leds((pps * 6) + digitStart,  (pps - 1 + (pps * 6)) + digitStart  ));

  if (x == 0) {
    segA = segB = segC = segD = segE = segF = CRGB(r, g, b);
    segG = CRGB::Black;
  }
  if (x == 1) {
    segB = segC = CRGB(r, g, b);
    segA = segD = segE = segF = segG = CRGB::Black;
  }
  if (x == 2) {
    segA = segB = segG = segE = segD = CRGB(r, g, b);
    segC = segF = CRGB::Black;
  }
  if (x == 3) {
    segA = segB = segG = segC = segD = CRGB(r, g, b);
    segE = segF = CRGB::Black;
  }
  if (x == 4) {
    segF = segG = segB = segC = CRGB(r, g, b);
    segA = segD = segE = CRGB::Black;
  }
  if (x == 5) {
    segA = segF = segG = segC = segD = CRGB(r, g, b);
    segB = segE = CRGB::Black;
  }
  if (x == 6) {
    segA = segF = segE = segD = segC = segG = CRGB(r, g, b);
    segB = CRGB::Black;
  }
  if (x == 7) {
    segA = segB = segC = CRGB(r, g, b);
    segD = segE = segF = segG = CRGB::Black;
  }
  if (x == 8) {
    segA = segB = segC = segD = segE = segF = segG = CRGB(r, g, b);
  }
  if (x == 9) {
    segG = segF = segA = segB = segC = segD = CRGB(r, g, b);
    segE = CRGB::Black;
  }
}

//-------------------------------------------------------------------------------------

void showTime() {

  // Digit 1 (seconds ones, far right)
  digitStart = digitStart1;
  displayDigit(secondsOnes);

  // Digit 2 (seconds tens)
  digitStart = digitStart2;
  displayDigit(secondsTens);

  // Digit 3 (minutes ones)
  digitStart = digitStart3;
  displayDigit(minutesOnes);

  // Digit 4 (minutes tens, far left)
  digitStart = digitStart4;
  displayDigit(minutesTens);

  // Dot separators
  segDots = CRGB(r, g, b);

  FastLED.show();

  // Print time to serial monitor

  Serial.print("Time = ");
  Serial.print(minutesTens);
  Serial.print(minutesOnes);
  Serial.print(":");
  Serial.print(secondsTens);
  Serial.println(secondsOnes);

}

//-------------------------------------------------------------------------------------
