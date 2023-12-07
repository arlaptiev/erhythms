/*
  Made by Gustavo Silveira, 2023.
  - This Sketch reads the Arduino's digital and analog ports and send midi notes and midi control change

  http://www.musiconerd.com
  http://www.youtube.com/musiconerd
  http://facebook.com/musiconerdmusiconerd
  http://instagram.com/musiconerd/
  http://www.gustavosilveira.net
  gustavosilveira@musiconerd.com

  If you are using for anything that's not for personal use don't forget to give credit.

  PS: Just change the value that has a comment like " // "

*/

// #define DEBUG 1 // comment if not debugging

#define USING_BUTTONS 1  // comment if not using buttons
#define USING_POTENTIOMETERS 1  // comment if not using potentiometers

/////////////////////////////////////////////
// LIBRARIES
#ifdef USING_POTENTIOMETERS
// include ResponsiveAnalogRead
#include <ResponsiveAnalogRead.h>  // [https://github.com/dxinteractive/ResponsiveAnalogRead](https://github.com/dxinteractive/ResponsiveAnalogRead)

#endif
// ---- //

/////////////////////////////////////////////
// BUTTONS
#ifdef USING_BUTTONS

// consts
const int N_BUTTONS = 2;                                //  total numbers of buttons
const int BUTTON_ARDUINO_PIN[N_BUTTONS] = { 3, 4 };  // pins of each button connected straight to the Arduino
const int DEBOUNCE_DELAY = 50;                          // the debounce time; increase if the output flickers

// states
char buttonState[N_BUTTONS][11]; // stores the current state in text
int buttonCVal[N_BUTTONS] = {};  // stores the button current value
int buttonPVal[N_BUTTONS] = {};  // stores the button previous value

// C-HIGH, P-LOW = button pressed
// C-LOW, P-HIGH = button released

// debounce
unsigned long lastDebounceTime[N_BUTTONS] = { 0 };  // the last time the output pin was toggled

#endif

/////////////////////////////////////////////
// POTENTIOMETERS
#ifdef USING_POTENTIOMETERS

// consts
const int N_POTS = 2;                            // total numbers of pots (slide & rotary)
const int POT_ARDUINO_PIN[N_POTS] = { A5, A6 };  // pins of each pot connected straight to the Arduino
const int TIMEOUT = 300;                         // Amount of time the potentiometer will be read after it exceeds the varThreshold
const int VARTHRESHOLD = 20;                     // Threshold for the potentiometer signal variation
const float SNAPMULTIPLIER = 0.01;               // (0.0 - 1.0) - Increase for faster, but less smooth reading (ResponsiveAnalogRead)
const int POTMIN = 10;
const int POTMAX = 1023;

// states
char potState[N_BUTTONS][11]; // stores the current state in text
int potCVal[N_POTS] = { 0 };  // Current value of the pot
int potPVal[N_POTS] = { 0 };  // Previous value of the pot

boolean potMoving[N_POTS] = { false };   // If the potentiometer is moving
unsigned long PTime[N_POTS] = { 0 };  // Previously stored time
unsigned long PTimer[N_POTS] = { 0 };  // Stores the time that has elapsed since the timer was reset

// Responsive Analog Read
ResponsiveAnalogRead responsivePot[N_POTS] = {};  // creates an array for the responsive pots. It gets filled in the Setup.

#endif


/////////////////////////////////////////////
// BUTTONS
#ifdef USING_BUTTONS

void update_buttons() {

  for (int i = 0; i < N_BUTTONS; i++) {

    buttonCVal[i] = digitalRead(BUTTON_ARDUINO_PIN[i]);  // read pins from arduino

    if (buttonCVal[i] == LOW) {
      strcpy(buttonState[i], "DOWN");  // Set state
    } else {
      strcpy(buttonState[i], "UP");    // Set state
    }

    if ((millis() - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
      if (buttonPVal[i] != buttonCVal[i]) {

        lastDebounceTime[i] = millis();
        buttonPVal[i] = buttonCVal[i];

        if (buttonCVal[i] == LOW) {
          strcpy(buttonState[i], "JUSTDOWN");  // Set state
        } else {
          strcpy(buttonState[i], "JUSTUP");    // Set state
        }

      }
    }

#if DEBUG
        Serial.print("Button ");
        Serial.print(i);
        Serial.print(": ");
        Serial.println(buttonState[i]);
#endif
  }
}

#endif

/////////////////////////////////////////////
// POTENTIOMETERS
#ifdef USING_POTENTIOMETERS

void update_potentiometers() {

  for (int i = 0; i < N_POTS; i++) {  // Loops through all the potentiometers

    int reading = analogRead(POT_ARDUINO_PIN[i]);
    responsivePot[i].update(reading);
    potCVal[i] = responsivePot[i].getValue();

    int potVar = abs(potCVal[i] - potPVal[i]);  // Difference between the current and previous state of the pot

    if (potVar > VARTHRESHOLD) {  // Opens the gate if the potentiometer variation is greater than the threshold
      PTime[i] = millis();        // Stores the previous time
    }

    PTimer[i] = millis() - PTime[i];  // Resets the timer 11000 - 11000 = 0ms

    if (PTimer[i] < TIMEOUT) {  // If the timer is less than the maximum allowed time it means that the potentiometer is still moving
      potMoving[i] = true;
      potPVal[i] = potCVal[i];    // Stores the current reading of the potentiometer to compare with the next
      strcpy(potState[i], "MOVING"); // Set state
    } else {
      potMoving[i] = false;
      strcpy(potState[i], "IDLE");   // Set state
    }

#if DEBUG
      Serial.print("Pot ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(potState[i]);
      Serial.print(" ");
      Serial.println(potCVal[i]);
#endif
  }
}

#endif


/////////////////////////////////////////////
// SETUP
void setup_controls() {

#ifdef USING_BUTTONS
  // Buttons
  // Initialize buttons with pull up resistors
  for (int i = 0; i < N_BUTTONS; i++) {
    pinMode(BUTTON_ARDUINO_PIN[i], INPUT_PULLUP);
  }

#endif

#ifdef USING_POTENTIOMETERS
  for (int i = 0; i < N_POTS; i++) {
    responsivePot[i] = ResponsiveAnalogRead(0, true, SNAPMULTIPLIER);
    responsivePot[i].setAnalogResolution(POTMAX);  // sets the resolution
  }
#endif
}


/////////////////////////////////////////////
// LOOP
void update_controls() {

#ifdef USING_BUTTONS
  update_buttons();
#endif

#ifdef USING_POTENTIOMETERS
  update_potentiometers();
#endif

}
