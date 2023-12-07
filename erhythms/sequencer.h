#include <AsyncTimer.h>

#include <vector>
#include <functional>

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

// sequence length
#define DEFAULT_SEQLENGTH 16

#define DEFAULT_TEMPO 20


// TODO FIX 2D NON-STATIC ARRAY THING!!!!


/**
 * MIDI clock generator. uClock by midilab seems like a great library to substitute this with.
 * Not available on some hardware yet. https://github.com/midilab/uClock/tree/main
*/
class Clock {
  public:
    Clock(): bpm(120), beatn(0) {}

    void start() {
      if (intervalId) {
        stop();
      }
      intervalId = t.setInterval([this]() { beatn++; beatHandler(beatn); }, 60000 / bpm);
    }

    void stop() {
      beatn = 0;
      t.cancelAll();
    }

    void setTempo(uint8_t bpm) {
      this->bpm = bpm;
      stop();
      start();
    }

    uint8_t getTempo() {
      return bpm;
    }

    void setBeatHandler(std::function<void(uint16_t)> aBeatHandler) {
      beatHandler = aBeatHandler;
    }

    void update() {
      t.handle();
      }

  private:
    AsyncTimer t;
    unsigned short intervalId;
    uint8_t bpm;
    uint beatn; 
    std::function<void(uint16_t)> beatHandler;
};


/**
 * Channel class. Contains a sequence and a pattern.
*/
class Channel {
  public:
    /**
     * Constructor with initializer list to initialize member variables.
     */
    Channel(): seqLength(16), muted(false), pos(15) {
      std::vector<bool> emptyPattern = { false };
      changeSequence(emptyPattern, 1, DEFAULT_SEQLENGTH);
    }

    Channel(int8_t seqLen): seqLength(seqLen), muted(false), pos(15) {
      std::vector<bool> emptyPattern = { false };
      changeSequence(emptyPattern, 1, seqLen);
    }

    /**
     * Offset the position in the sequence by the given amount.
     */
    uint8_t offsetPos(int8_t offset) {
        pos = (pos + offset) % seqLength;
        return pos;
    }

    /**
     * Get the current position in the sequence.
     */
    uint8_t getPos() { return pos; }

    /**
     * Mute or unmute the channel.
     */
    bool muteToggle() {
        muted = !muted;
        return muted;
    }

    /**
     * Generate and update the sequence based on the given pattern and sequence length.
     */
    std::vector<bool> changeSequence(std::vector<bool> newPattern, uint8_t newPatLength, uint8_t newSeqLength) {
      // Use 'this->' to distinguish between member variables and parameters
      this->patLength = newPatLength;
      this->seqLength = newSeqLength;
      
      // Delete old and allocate a new pattern array
      pattern.clear();
      // Copy elements from the parameter pattern to the member variable pattern
      for (uint8_t i = 0; i < patLength; ++i) {
          pattern.push_back(newPattern.at(i));
      }

      // Delete old and allocate a new sequence array
      sequence.clear();
      if (patLength < seqLength) {
          // Repeat pattern to fill seqLength
          for (uint8_t i = 0; i < seqLength; ++i) {
            sequence.push_back(newPattern.at(i % patLength));
          }
      } else {
          // Truncate pattern to fit seqLength
          for (uint8_t i = 0; i < seqLength; ++i) {
            sequence.push_back(newPattern.at(i));
          }
      }

      // Adjust the position if necessary
      if (pos >= seqLength) {
          pos = seqLength - 1;
      }

      // Return the generated sequence
      return sequence;
    }

    std::vector<bool> changeSequence(uint8_t newSeqLength) {
      changeSequence(pattern, patLength, newSeqLength);
    }

    std::vector<bool> changeSequence(std::vector<bool> newPattern, uint8_t newPatLength) {
      changeSequence(newPattern, newPatLength, seqLength);
    }


    /**
     * Get the current sequence.
     */
    std::vector<bool> getSequence() { return sequence; }

    uint8_t getSequenceLength() { return seqLength; }

    /**
     * Step the sequencer forward one step. Returns true if the step is a trigger.
     */
    bool step() {
      offsetPos(1);
      return sequence[pos] && !muted;
    }

private:
    std::vector<bool> pattern;
    uint8_t patLength;
    std::vector<bool> sequence;
    uint8_t seqLength;
    uint8_t pos;     // where in the current seq we are
    bool muted;
};



/*
 * Sequencer with MIDI and uClock.
 */
class MIDISequencer
{
  public:
  
  // constructors
  MIDISequencer(uint8_t nChannels): nChannels(nChannels) {
    channels = new Channel[nChannels];
  }

  MIDISequencer(uint8_t nChannels, uint8_t seqLeng): nChannels(nChannels), maxSeqLength(seqLeng) {
    channels = new Channel[nChannels];
    for (uint8_t i = 0; i < nChannels; ++i) {
      channels[i] = Channel(seqLeng);
    }
  }

  MIDISequencer(uint8_t nChannels, uint8_t seqLeng, uint8_t maxSeqLeng): nChannels(nChannels), maxSeqLength(maxSeqLeng) {
    channels = new Channel[nChannels];
    for (uint8_t i = 0; i < nChannels; ++i) {
      channels[i] = Channel(seqLeng);
    }
  }

  /**
   * Set the function to call at the top of a beat.
   * Use std::function to allow for other class's member functions and other.
  */
  // void setBeatHandler(void (*aBeatHandler)(uint8_t* pos, bool** allSequences)) {
  //   beatHandler = aBeatHandler;
  // }
  void setBeatHandler(void (*aBeatHandler)(uint8_t* pos, std::vector<std::vector<bool>>& allSequences)) {
    beatHandler = aBeatHandler;
  }
  
  /**
   * Set the function to call to trigger notes/samples.
   * Use std::function to allow for other class's member functions and other.
  */
  void setTriggerHandler(void (*aTriggerHandler)(bool* allTrigs)) {
    triggerHandler = aTriggerHandler;
  }
  
  /*
   * Set the tempo in beats per minute
   */
  void setTempo( float bpm ) { clock.setTempo(bpm); }

  uint8_t getTempo() { return clock.getTempo(); }

  void setLength( uint8_t length ) { 
    for (uint8_t i = 0; i < nChannels; ++i) {
      channels[i].changeSequence(length);
    }
  }

  /**
   * Get the length of the sequence.
   * Assumes all channels have the same sequence length.
  */
  uint8_t getLength() { 
    return channels[0].getSequenceLength();
   }

  /**
   * Offset the position in the sequence by the given amount.
   * Assumes all channels have the same sequence length.
  */
  void offsetLength( int8_t offset ) { 
    uint8_t length = getLength();
    if (length + offset > maxSeqLength) {
      offset = maxSeqLength - length;
    }
    if (length + offset < 1) {
      offset = 1 - length;
    }
    setLength(length + offset);
  }

  // bool* step(uint16_t beatn) {
  void step(uint16_t beatn) {
    const uint8_t seqLength = getLength();

    bool* allTrigs = new bool[nChannels];
    uint8_t* allPos = new uint8_t[nChannels];
    // bool** allSequences = new bool[nChannels][seqLength];
    // <std::vector<bool> allSequences = new bool[nChannels][seqLength];
    std::vector<std::vector<bool>> allSequences(nChannels, std::vector<bool>(seqLength));

    for (uint8_t i = 0; i < nChannels; ++i) {
      allTrigs[i] = channels[i].step();
      allPos[i] = channels[i].getPos();
      allSequences[i] = channels[i].getSequence();
    }

    // trigger beatHandler
    if (beatHandler) { beatHandler(allPos, allSequences); }

    // trigger triggerHandler
    if (triggerHandler) { triggerHandler(allTrigs); }

    // return allTrigs;
  }

  void start() { 
    clock.setBeatHandler([this](uint16_t beatn) { Serial.println("BEAT IN CLOCK"); step(beatn); });
    clock.setTempo(DEFAULT_TEMPO);
    clock.start(); 
    }

  void stop() { clock.stop(); }

  void update() { clock.update(); }


  uint8_t nChannels;
  Channel* channels;


  private:    
    Clock clock;
    uint8_t maxSeqLength;
    void (*triggerHandler)(bool* allTrigs);
    // void (*beatHandler)(uint8_t* pos, bool** allSequences);
    void (*beatHandler)(uint8_t* pos, std::vector<std::vector<bool>>& allSequences);

   
};


// // The callback function wich will be called by Clock each Pulse of 96PPQN clock resolution.
// void ClockOut96PPQN(uint16_t beatn) {
//   // Send MIDI_CLOCK to external gears
//   Serial.println("tick");
//   Serial.println(beatn);
//   // bool* seq = ch.getSequence();
//   // uint8_t l = ch.getSequenceLength();
//   // for (int i = 0; i < l; i++){
//   //   Serial.print(seq[l]);
//   // }
//   // Serial.println();
//   std::vector<bool> pattern = {true, false, true};
//   std::vector<bool> seq = ch.changeSequence(pattern, 3, 16);


//   Serial.println(ch.step());
// }
void beatH(uint8_t* pos, std::vector<std::vector<bool>>& allSequences) {
  // Send MIDI_CLOCK to external gears
  Serial.println("tick");
}
void trigH(bool* allTrigs) {
  // Send MIDI_CLOCK to external gears
  for (int i = 0; i < sizeof(allTrigs); i++) {
    // Serial.println(allTrigs[i]);
  }
}

MIDISequencer s(2);

void setup() {

  // Initialize serial communication at 31250 bits per second, the default MIDI serial speed communication:
  Serial.begin(31250);
  std::vector<bool> pattern = {true, false, true};
  s.channels[0].changeSequence(pattern, 3);

  // Inits the clock
  // uClock.init();
  // // Set the callback function for the clock output to send MIDI Sync message.
  s.setBeatHandler(beatH);
  s.setTriggerHandler(trigH);
  // // // Set the callback function for MIDI Start and Stop messages.
  // // // uClock.setOnClockStartOutput(onClockStart);  
  // // // uClock.setOnClockStopOutput(onClockStop);
  // // // Set the clock BPM to 126 BPM
  s.setTempo(20);

  // // // Starts the clock, tick-tac-tick-tac...
  s.start();

}


// // The callback function wich will be called when clock starts by using Clock.start() method.
// void onClockStart() {
//   Serial.write(MIDI_START);
// }

// // The callback function wich will be called when clock stops by using Clock.stop() method.
// void onClockStop() {
//   Serial.write(MIDI_STOP);
// }


// class MIDISequencer
// {
//   public:
  
//   // constructor
//   MIDISequencer(): enabled(false), seq_id(0), pos(0) {}

//   // which pattern to play  
//   void setSeqId(uint8_t id) { seq_id = id % seq_count; }
//   uint8_t getSeqId() { return seq_id; }
//   uint8_t getSeqCount() { return seq_count; }
  
//   // set the function to call at the top of a beat
//   void setBeatHandler(void (*aBeatHandler)(uint8_t beatnum)) {
//     beatHandler = aBeatHandler;
//   }
  
//   // set the function to call to trigger drum samples
//   void setTriggerHandler(void (*aTriggerHandler)(bool bd, bool sd,bool ch, bool oh)) {
//     triggerHandler = aTriggerHandler;
//   }
  
//   //   
//   void setBPM( float bpm ) {
//     bpm = bpm;
//     per_beat_millis = 1000 * 60 / bpm / 4;
//   }
  
//   void update() {
//     uint32_t now = millis();
//     if( now - last_beat_millis < per_beat_millis ) { return; }
    
//     last_beat_millis = now;

//     // trigger beatHandler
//     if( beatHandler ) { beatHandler( pos ); }

    
//     uint16_t* seq = seqs[ seq_id ];
//     uint16_t seq_bd = seq[0];
//     uint16_t seq_sd = seq[1];
//     uint16_t seq_ch = seq[2];
//     uint16_t seq_oh = seq[3];

//     // pos is current position in sequence
//     pos = (pos+1) % seq_len; // seq_len is int16_t
   
//     bool bd_on = seq_bd & (1<<pos); // 0 if none, non-zero if trig
//     bool sd_on = seq_sd & (1<<pos); // 0 if none, non-zero if trig
//     bool ch_on = seq_ch & (1<<pos); // 0 if none, non-zero if trig
//     bool oh_on = seq_oh & (1<<pos); // 0 if none, non-zero if trig

//     if( triggerHandler ) { triggerHandler(bd_on, sd_on, ch_on, oh_on ); }
//   }

//   private:
//     bool    enabled;       // is seq playing or not
//     float   bpm;           // our arps per minute
//     uint8_t foop;
//     uint8_t seq_id;        // which arp we using
//     uint8_t pos;           // where in current seq we are
//     uint16_t per_beat_millis; // = 1000 * 60 / bpm;
//     uint32_t last_beat_millis;
    
//     void (*triggerHandler)(bool bd, bool sd, bool ch, bool oh) = nullptr;
//     void (*beatHandler)(uint8_t beat_num) = nullptr;

//     static const int seq_len = 16;  // number of trigs in an seq
//     static const int seq_inst_num = 4;
//     static const uint8_t seq_count= 3;   // how many seqs in "seqs"
    
//     uint16_t seqs[seq_count][seq_inst_num] = {
//       {
//         0b1000000010000001, // bd
//         0b0000100000001000, // sd
//         0b1010101010101010, // ch
//         0b0000000000000001, // oh
//       },
//       {
//         0b1000000010000001, // bd
//         0b0000100000001000, // sd
//         0b1111111011111110, // ch
//         0b0000000100000001, // oh
//       },
//       {
//         0b1100110011001001, // bd
//         0b0000100000001000, // sd
//         0b0011001100110011, // ch
//         0b1000000110000001, // oh
//       },
//     };
   
// };