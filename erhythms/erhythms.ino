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

MIDISequencer seq(6);

void beatH(uint8_t* pos, bool** allSequences, uint8_t nChannels, uint8_t seqLength, uint16_t beatnum) {
  // Send MIDI_CLOCK to external gears
  Serial.println("tick");
}
void trigH(bool* allTrigs, uint8_t nChannels) {
  // Send MIDI_CLOCK to external gears
  for (int i = 0; i < nChannels; i++) {
    Serial.println(allTrigs[i]);
  }
}


void setup() {

  // Initialize serial communication at 31250 bits per second, the default MIDI serial speed communication:
  Serial.begin(31250);

  bool pattern[] = {true, false, true};
  seq.channels[0].changeSequence(pattern, 3);
  seq.setBeatHandler(beatH);
  seq.setTriggerHandler(trigH);
  seq.start();

}


void loop() {
  seq.update();
  // update controls
  // channelBtns.update();
  // knobs.update();
}
