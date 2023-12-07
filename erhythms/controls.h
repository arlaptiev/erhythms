#include <EventButton.h>       // https://github.com/Stutchbury/EventButton/tree/main
#include <EventAnalog.h>       // https://github.com/Stutchbury/EventAnalog/tree/main
// #include <EncoderButton.h>     // https://github.com/Stutchbury/EncoderButton/tree/main

#include <vector>

// button settings (more in documentation)
#define LONGCLICK_DUR 100       // Long Press fires at this timeout. Later, Long Click fires at release
#define MULTICLICK_INT 1        // Click and Double Click fire at this timeout. Alternatively, for quick Click, Click fires at release after this timeout
#define IDLE_TIMEOUT 5000       // Idle fires at this timeout

// analog settings (more in documentation)
#define NUM_INCREMENTS 25       // Number of increments for analogs
#define NUM_NEGINCREMENTS 25    // Number of negative increments for analogs
#define NUM_NEPOSCREMENTS 25    // Number of negative increments for analogs
#define START_VALUE 0           // Start value for analogs
#define START_BOUNDARY 200      // Start boundary for analogs
#define END_BOUNDARY 100        // End boundary for analogs
#define RATE_LIMIT 0            // Rate limit for analogs

// default states
#define IDLE 0



#ifdef DEBUG
// debugging button events
void click(EventButton& eb) { Serial.print(eb.userId()); Serial.println("B CLICKED"); }
void click2(EventButton& eb) { Serial.print(eb.userId()); Serial.println("B DOUBLE CLICKED"); }
void longp(EventButton& eb) { Serial.print(eb.userId()); Serial.print("B LONG PRESSED ("); Serial.print(eb.longPressCount()); Serial.println(")"); }
void longc(EventButton& eb) { Serial.print(eb.userId()); Serial.println("B LONG CLICKED"); }
void press(EventButton& eb) { Serial.print(eb.userId()); Serial.println("B PRESSED"); }
void rel(EventButton& eb) { Serial.print(eb.userId()); Serial.println("B RELEASED"); }
void idle(EventButton& eb) { Serial.print(eb.userId()); Serial.println("B IDLE"); }

// debugging analog
void changed(EventAnalog& ea) { Serial.print(ea.userId()); Serial.print("A CHANGED: "); Serial.println(ea.position()); }
void idlea(EventAnalog& ea) { Serial.print(ea.userId()); Serial.println("A IDLE"); }

// debugging encoder
// void enc(EncoderButton& eb) { Serial.print(eb.userId()); Serial.print("E TURNED: "); Serial.println(eb.position()); }
// void clicke(EncoderButton& eb) { Serial.print(eb.userId()); Serial.println("E CLICKED"); }
// void pressturn(EncoderButton& eb) { Serial.print(eb.userId()); Serial.print("E PRESSED AND TURNED: "); Serial.println(eb.pressedPosition()); }
#endif


/*
 * BUTTONS CLASS
 */
class Buttons {
  public:
    // Constructors

    /**
     * Construct a buttons object with default settings
     */
    Buttons(
      byte* pins, uint8_t n,
      unsigned int defaultState = IDLE,
      unsigned int longClickDuration = LONGCLICK_DUR,
      unsigned int multiClickInterval = MULTICLICK_INT,
      unsigned int idleTimeout = IDLE_TIMEOUT
      ): nButtons(n) {
      // initialize the buttons
      for (int i = 0; i < nButtons; i++) { b.emplace_back(EventButton(pins[i])); }    // fill up an array
      init(defaultState);                                                             // set states
      setConfigs(longClickDuration, multiClickInterval, idleTimeout);                 // set configs
      }


    // Public variables
    uint8_t nButtons;
    std::vector<EventButton> b;


    // Public methods
    void update() {
      // run update
      for (int i = 0; i < nButtons; i++) { b[i].update(); }
    }

    void setStates(unsigned int state) {
      for (int i = 0; i < nButtons; i++) { b[i].setUserState(state); }
    }

    void setConfigs(
      unsigned int longClickDuration = LONGCLICK_DUR,
      unsigned int multiClickInterval = MULTICLICK_INT,
      unsigned int idleTimeout = IDLE_TIMEOUT
      ) {
      // set configs
      for (int i = 0; i < nButtons; i++) { 
        b[i].setLongClickDuration(longClickDuration);       // setting description above
        b[i].setMultiClickInterval(multiClickInterval);     // setting description above
        b[i].setIdleTimeout(idleTimeout);                   // setting description above
        }
      }


  private:
    void init(unsigned int state = 0) {
        // run init
        for (int i = 0; i < nButtons; i++) {
          b[i].setUserId(i);                                  // set User Id to identify the event target
          b[i].setUserState(state);                           // set User State
  #ifdef DEBUG
          b[i].setClickHandler(click);
          b[i].setDoubleClickHandler(click2);
          b[i].setLongClickHandler(longc);
          b[i].setLongPressHandler(longp, true);              // true for Long Press repeat firing at LONGCLICK_DUR interval
          b[i].setPressedHandler(press);
          b[i].setReleasedHandler(rel);
          b[i].setIdleHandler(idle);
  #endif
          }
      }
  };



/*
 * ANALOGS CLASS
 */
class Analogs {
  public:
    // Constructor
    /**
     * Construct an analogs object with default settings
     */
    Analogs(
      byte* pins, uint8_t n,
      unsigned int defaultState = IDLE,
      unsigned int numIncrements = NUM_INCREMENTS
      ): nAnalogs(n) {
      // initialize the analogs
      for (int i = 0; i < nAnalogs; i++) { a.emplace_back(EventAnalog(pins[i])); }    // fill up an array
      init(defaultState);                                                             // set states
      setConfigs(numIncrements, RATE_LIMIT);                                           // set configs
      }

    /**
     * Construct an analogs object with numIncrements settings
     */
    Analogs(
      byte* pins, uint8_t n,
      unsigned int defaultState,
      unsigned int numIncrements,
      unsigned int rateLimit
      ): nAnalogs(n) {
      // initialize the analogs
      for (int i = 0; i < nAnalogs; i++) { a.emplace_back(EventAnalog(pins[i])); }    // fill up an array
      init(defaultState);                                                             // set states
      setConfigs(numIncrements, rateLimit);                                           // set configs
      }

    /**
     * Construct an analogs object with numNegativeIncrements and numPositiveIncrements settings
     */
    Analogs(
      byte* pins, uint8_t n,
      unsigned int defaultState,
      unsigned int numNegativeIncrements,
      unsigned int numPositiveIncrements,
      unsigned int rateLimit
      ): nAnalogs(n) {
      // initialize the analogs
      for (int i = 0; i < nAnalogs; i++) { a.emplace_back(EventAnalog(pins[i])); }    // fill up an array
      init(defaultState);                                                             // set states
      setConfigs(numNegativeIncrements, numPositiveIncrements, rateLimit);      // set configs
      }


    // Public variables
    uint8_t nAnalogs;
    std::vector<EventAnalog> a;


    // Public methods
    void update() {
      // run update
      for (int i = 0; i < nAnalogs; i++) { a[i].update(); }
    }

    void setStates(unsigned int state) {
      for (int i = 0; i < nAnalogs; i++) { a[i].setUserState(state); }
    }

    void setConfigs(
      unsigned int numIncrements,
      unsigned int rateLimit
      ) {
      // set configs
      for (int i = 0; i < nAnalogs; i++) { 
        a[i].setNumIncrements(numIncrements);               // setting description above
        a[i].setStartValue(START_VALUE);
        a[i].setStartBoundary(START_BOUNDARY);
        a[i].setEndBoundary(END_BOUNDARY);
        a[i].setRateLimit(rateLimit);
      }
    }

    void setConfigs(
      unsigned int numNegativeIncrements,
      unsigned int numPositiveIncrements,
      unsigned int rateLimit
      ) {
      // set configs
      for (int i = 0; i < nAnalogs; i++) { 
        a[i].setNumNegativeIncrements(numNegativeIncrements);         // setting description above
        a[i].setNumPositiveIncrements(numPositiveIncrements);         // setting description above
        a[i].setRateLimit(rateLimit);
      }
    }


  private:
    void init(unsigned int state = 0) {
        // run init
        for (int i = 0; i < nAnalogs; i++) {
          a[i].setUserId(i);                                  // set User Id to identify the event target
          a[i].setUserState(state);                           // set User State
  #ifdef DEBUG
          a[i].setChangedHandler(changed);
          a[i].setIdleHandler(idlea);
  #endif
          }
      }
};



/*
 * ENCODERBUTTONS CLASS
 */
// class EncoderButtons {
//   public:
//     // Constructors

//     /**
//      * Construct an encoder object without a button with default settings
//      */
//     EncoderButtons(
//       byte (*pins)[2], uint8_t n,
//       unsigned int defaultState = 0
//       ): nEncoders(n) {
//       // initialize the encoders
//       for (int i = 0; i < nEncoders; i++) { e.emplace_back(EncoderButton(pins[i][0], pins[i][1])); }     // fill up an array
//       setStates(defaultState);                                                                           // set states
//       setConfigs(LONGCLICK_DUR, MULTICLICK_INT, IDLE_TIMEOUT);
//     }

//     /**
//      * Construct an encoder object with a button with default settings
//      */
//     EncoderButtons(
//       byte (*pins)[3], uint8_t n,
//       unsigned int defaultState = IDLE,
//       unsigned int longClickDuration = LONGCLICK_DUR,
//       unsigned int multiClickInterval = MULTICLICK_INT,
//       unsigned int idleTimeout = IDLE_TIMEOUT
//       ): nEncoders(n) {
//       // initialize the encoders
//       for (int i = 0; i < nEncoders; i++) { e.emplace_back(EncoderButton(pins[i][0], pins[i][1], pins[i][2])); }     // fill up an array
//       setConfigs(longClickDuration, multiClickInterval, idleTimeout);
//     }


//     // Public variables
//     uint8_t nEncoders;
//     std::vector<EncoderButton> e;


//     // Public methods
//     void update() {
//       // run update
//       for (int i = 0; i < nEncoders; i++) { e[i].update(); }
//     }

//     void setStates(unsigned int state) {
//       for (int i = 0; i < nEncoders; i++) { e[i].setUserState(state); }
//     }

//     void setConfigs(
//       unsigned int longClickDuration = LONGCLICK_DUR,
//       unsigned int multiClickInterval = MULTICLICK_INT,
//       unsigned int idleTimeout = IDLE_TIMEOUT
//       ) {
//       // set configs
//       for (int i = 0; i < nEncoders; i++) {
//         e[i].setLongClickDuration(longClickDuration);       // setting description above
//         e[i].setMultiClickInterval(multiClickInterval);     // setting description above
//         e[i].setIdleTimeout(idleTimeout);                   // setting description above
//       }
//     }


//  private:
//     void init(unsigned int state = 0) {
//         // run init
//         for (int i = 0; i < nEncoders; i++) {
//           e[i].setUserId(i);                                  // set User Id to identify the event target
//           e[i].setUserState(state);                           // set User State
//   #ifdef DEBUG
//           e[i].setEncoderHandler(enc);
//           e[i].setClickHandler(clicke);
//           e[i].setEncoderPressedHandler(pressturn);
//   #endif
//           }
//       }

// };
