/*
  MIDI note player

  This sketch shows how to use the serial transmit pin (pin 1) to send MIDI note data.
  If this circuit is connected to a MIDI synth, it will play the notes
  F#-0 (0x1E) to F#-5 (0x5A) in sequence.

  The circuit:
  - digital in 1 connected to MIDI jack pin 5
  - MIDI jack pin 2 connected to ground
  - MIDI jack pin 4 connected to +5V through 220 ohm resistor
  - Attach a MIDI cable to the jack, then to a MIDI synth, and play music.

  created 13 Jun 2006
  modified 13 Aug 2012
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Midi
*/

// Use pins 0/1, 12/13, 16/17 and 28/29. Crash otherwise
#define TX 14
#define RX 28

void setup() {
  // Set MIDI baud rate:
  Serial1.setTX(28);
  Serial1.setRX(13);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(31250);
  Serial1.begin(31250);
}

void loop() {
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte);
  }

  // play notes from F#-0 (0x1E) to F#-5 (0x5A):
  for (int note = 0x1E; note < 0x5A; note++) {
    // Serial.println("SEND");
    //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
    // Serial.println("SEND1");
    noteOn(0x90, note, 0x45);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    //Note on channel 1 (0x90), some note value (note), silent velocity (0x00):
    // Serial.println("SEND2");
    digitalWrite(LED_BUILTIN, LOW);
    noteOn(0x90, note, 0x00);
    delay(100);
  }
}

// plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that
// data values are less than 127:
void noteOn(int cmd, int pitch, int velocity) {
  Serial1.write(cmd);
  Serial1.write(pitch);
  Serial1.write(velocity);
  // Serial.write(cmd);
  // Serial.write(pitch);
  // Serial.write(velocity);
}



// #include "MIDIUSB.h"

// void noteOn(byte channel, byte pitch, byte velocity) {
//   midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
//   MidiUSB.sendMIDI(noteOn);
//   MidiUSB.flush();
// }

// void noteOff(byte channel, byte pitch, byte velocity) {
//   midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
//   MidiUSB.sendMIDI(noteOff);
//   MidiUSB.flush();
// }

// void controlChange(byte channel, byte control, byte value) {
//   midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
//   MidiUSB.sendMIDI(event);
//   MidiUSB.flush();
// }

