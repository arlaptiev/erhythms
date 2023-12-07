#define DEBUG 1

// #include "controls.h"
#include "sequencer.h"
// #include "midi.h"
// #include "euclidean.h"

byte BTN_PINS[] = { 3, 4 };
byte ANALOG_PINS[] = { A1, A2 };
byte ENCODER_PINS[][2] = { {1, 2}, {1, 2} };

// Initialize the controls
// Buttons channelBtns(
//   BTN_PINS, sizeof(BTN_PINS)
// );

// Analogs knobs(
//   ANALOG_PINS, sizeof(ANALOG_PINS)
// );

// void setup() {
//   // Initialize serial communication at 31250 bits per second, the default MIDI serial speed communication:
//   Serial.begin(31250);
// }

void loop() {
  s.update();
  // update controls
  // channelBtns.update();
  // knobs.update();
}
