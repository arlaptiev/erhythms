#include <AsyncTimer.h>

#include <vector>
#include <functional>

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

// sequence length
#define DEFAULT_SEQLENGTH 16
#define MAX_SEQLENGTH 16
#define MAX_CHANNELS 16

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

    // TODO FIX CRASHES THE STARTUP OF THE CHIP FOR SOME REASON
    void setTempo(uint8_t bpm) {
      this->bpm = bpm;
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
      bool emptyPattern[] = { false };
      changeSequence(emptyPattern, 1, DEFAULT_SEQLENGTH);
    }

    Channel(int8_t seqLen): seqLength(seqLen), muted(false), pos(15) {
      bool emptyPattern[] = { false };
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
    bool* changeSequence(bool newPattern[], uint8_t newPatLength, uint8_t newSeqLength) {
      // Use 'this->' to distinguish between member variables and parameters
      this->patLength = newPatLength;
      this->seqLength = newSeqLength;
      
      // Copy elements from the parameter pattern to the member variable pattern
      for (uint8_t i = 0; i < patLength; ++i) {
          pattern[i] = newPattern[i];
      }

      if (patLength < seqLength) {
        // Repeat pattern to fill seqLength
        for (uint8_t i = 0; i < seqLength; ++i) {
          sequence[i] = newPattern[i % patLength];
        }
      } else {
        // Truncate pattern to fit seqLength
        for (uint8_t i = 0; i < seqLength; ++i) {
          sequence[i] = newPattern[i];
        }
      }

      // Adjust the position if necessary
      if (pos >= seqLength) {
          pos = seqLength - 1;
      }

      // Return the generated sequence
      return sequence;
    }

    bool* changeSequence(uint8_t newSeqLength) {
      changeSequence(pattern, patLength, newSeqLength);
    }

    bool* changeSequence(bool newPattern[], uint8_t newPatLength) {
      changeSequence(newPattern, newPatLength, seqLength);
    }


    /**
     * Get the current sequence.
     */
    bool* getSequence() { return sequence; }

    uint8_t getSequenceLength() { return seqLength; }

    /**
     * Step the sequencer forward one step. Returns true if the step is a trigger.
     */
    bool step() {
      offsetPos(1);
      return sequence[pos] && !muted;
    }

private:
    bool pattern[MAX_SEQLENGTH];
    uint8_t patLength;
    bool sequence[MAX_SEQLENGTH];
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
  void setBeatHandler(void (*aBeatHandler)(uint8_t* pos, bool** allSequences, uint8_t nChannels, uint8_t seqLength, uint16_t beatnum)) {
    beatHandler = aBeatHandler;
  }
  
  /**
   * Set the function to call to trigger notes/samples.
   * Use std::function to allow for other class's member functions and other.
  */
  void setTriggerHandler(void (*aTriggerHandler)(bool* allTrigs, uint8_t nChannels)) {
    triggerHandler = aTriggerHandler;
  }
  
  /*
   * Set the tempo in beats per minute
   */
  void setTempo( float bpm ) { clock.setTempo(bpm); }

  float getTempo() { return clock.getTempo(); }

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

  bool* step(uint16_t beatnum) {
    const uint8_t seqLength = getLength();

    bool* allTrigs = new bool[nChannels];
    uint8_t* allPos = new uint8_t[nChannels];
    bool** allSequences = new bool*[MAX_SEQLENGTH];

    for (uint8_t i = 0; i < nChannels; ++i) {
      allTrigs[i] = channels[i].step();
      allPos[i] = channels[i].getPos();
      allSequences[i] = channels[i].getSequence();
    }

    // trigger beatHandler
    if (beatHandler) { beatHandler(allPos, allSequences, nChannels, seqLength, beatnum); }

    // trigger triggerHandler
    if (triggerHandler) { triggerHandler(allTrigs, nChannels); }

    return allTrigs;
  }

  void start() { 
    clock.setBeatHandler([this](uint16_t beatnum) { step(beatnum); });
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
    void (*triggerHandler)(bool* allTrigs, uint8_t nChannels);
    void (*beatHandler)(uint8_t* pos, bool** allSequences, uint8_t nChannels, uint8_t seqLength, uint16_t beatnum);

   
};

