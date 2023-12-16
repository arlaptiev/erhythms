#define DEBUG 1

#include "controls.h"
#include "sequencer.h"
// #include "midi.h"
// #include "euclidean.h"

#define TX 14
#define RX 28

byte GENERALBTN_PINS[] = { 18, 21, 7};
byte CHANNELBTN_PINS[] = { 5, 4, 3, 2, 1, 0 };
byte ANALOG_PINS[] = { A1, A2 };
byte ENCODER_PINS[][2] = { {26, 27}, {16, 17} };

// Initialize the controls
Buttons generalBtns( GENERALBTN_PINS, 3 );
Buttons channelBtns( CHANNELBTN_PINS, 6 );

// Analogs seqKnobs( ANALOG_PINS, 2 );

EncoderButtons seqKnobs( ENCODER_PINS, 2 );

// Initialize sequencer
// MIDISequencer seq(6);


/**
 * STATES
*/

#define PLAYING 1
#define STOPPED 0
#define INTERNAL 0
#define EXTERNAL 1
#define MIDI 1
#define CV 0

// Channel* selectedChannel;
byte playState = STOPPED;
byte clockState = INTERNAL; // GENERALBTN_PINS[3].state TODO!!
byte extClockSource = MIDI;

/**
 * FEATURES
*/

// void togglePlayState() {
//   if (playState == PLAYING) {
//     playState = STOPPED;
//     seq.stop();
//   } else {
//     playState = PLAYING;
//     seq.start();
//   }
// }

void changeExtClockSource(byte src) {
  extClockSource = src;
}

void changeClockState(byte state) {
  clockState = state;
}

// void selectChannel(byte channelId) {
//   selectedChannel = &seq.channels[channelId];
// }

// void setSequenceLength(uint8_t length) {
//   seq.setLength(length);
// }

// void offsetLength(uint8_t offset) {
//   seq.offsetLength(offset);
// }


void beatH(uint8_t* pos, bool** allSequences, uint8_t nChannels, uint8_t seqLength, uint16_t beatnum) {
  // Send MIDI_CLOCK to external gears
  // Serial.println("tick");
}
void trigH(bool* allTrigs, uint8_t nChannels) {
  // Send MIDI_CLOCK to external gears
  for (int i = 0; i < nChannels; i++) {
    // Serial.println(allTrigs[i]);
  }
}


void setup() {

  // Initialize serial communication at 31250 bits per second, the default MIDI serial speed communication:
  // Serial1.setTX(TX);
  // Serial1.setRX(RX);
  // Serial1.begin(31250);
  Serial.begin(31250);

  // Set up inputs
  // encoders
  // pinMode(16, INPUT_PULLUP);
  // pinMode(17, INPUT_PULLUP);
  // clock
  pinMode(7, INPUT_PULLDOWN);
  pinMode(28, INPUT_PULLUP);

  bool pattern[] = {true, false, true};
  // seq.channels[0].changeSequence(pattern, 3);
  // seq.setBeatHandler(beatH);
  // seq.setTriggerHandler(trigH);
  // seq.start();

}


void loop() {
  // Serial.print("7: ");
  // Serial.println(digitalRead(7));
  // Serial.print("28: ");
  // Serial.println(digitalRead(28));
  // seq.update();
  // // update controls
  channelBtns.update();
  generalBtns.update();
  seqKnobs.update();
}
