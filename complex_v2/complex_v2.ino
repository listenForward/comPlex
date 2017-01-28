/*
   (cc) Patrick Richardson, 2016,
   sketch to blink if/when a MIDI IN port (on uArt RX pin) receives a specific MIDI message of type

   TODO:
      relimiante reduncance in UO led counter-handlers,  
        pass Pointer to Which pinUO to call, or just define "relevant pin" as argument...variablistically ?

        int i = advance(i,boundary, pin){
          switch(i){
            case(i < ...):
              digitalWrite(pin,LOW)
              break
              
            case(i > ...)
              digitalWrite(pin,HIGH);
          }
          
          switch(i)
           
        }
*/





/*
  ##################################################
  CONFIG
  ##################################################
*/
// UI
#define pinLED_myQ  13
#define pinLED_seam 14


// MIDI event UO lighting times.
#define blinkPs 6   // clock ticks;   hold for ~ clockPulses  after ON, then turn OFF...
#define blinkTs 10  // mSec       ;   hold for ~ mSec         after ON, then turn OFF...

// myQuarterNote UO
#define myPPQ   24
int iTick     = 0;

// utility timing
int myPPQuant = 12;


/* 
---------------
  MIDI Loop object
----------------
*/

// timing
typedef struct  // define structure and data-object classes within "myLoop" object
{
  unsigned long i;
  unsigned long L;
  unsigned long seamIn;
  unsigned long seamOut;

  char stateNow[20];
  char stateWas[20];
} someLoop,
  *myLoop;


 /*
    myLoop.stateList[] = {  // LoopPoints?              Events ?            Tick Time ?
        
        "open",                   // nullify                  empty()             no 
        "empty",                  // defines                  empty()             tick++
        "stopped",                // loop size,               hold                no
        "playing",                // defined                  play,hold           tick++    
        "recording",              // reset, append, close     remove(All),        tick++
        "new ++",                 // defined                  play(), add()       tick++
        "new -+",                 // defined                  remove(), add(),    tick++
        "resizing"                // index/append/shift       play(), hold        tick++
    }; 
  */

  myLoop.i = 0;
  myLoop.seamIn = 0;
  myLoop.seamOut;
  myLoop.stateNow  = "open";
  myLoop.stateWas  = "open";     // for undo ?

typedef struct{
   int j;
   unsigned long when[] = {};
   unsigned long what[] = {};

} someListerator;


// LOOP DATA      ...store midi event 
listerator.j    = 0;  // index placeHolder [0,N], to iterate through listerator; if(myLoop.i == listerator[j].when {do("what") }
/*  
 myLoop.evenTs      an N-long array of events
    .when           a tickIndex to play 
    .what.            
      .status       MIDI byte; message type and channel
      .data1        MIDI byte, varies
      .data2        MIDI byet, varies
 */


/*
    MIDI CC IDs for loop control
*/

//  general purpose Buttons, single-action:
#define myCCs.tap       64

// per loop, play
#define myCCs.reStart   80
#define myCCs.halt      81
#define myCCs.proceed   82
#define myCCs.reSeam    83

#define myCCs.empty
#define myCCs.destroy    

//  undefined on/off's      latch/momentary
#define myCCs.newRec    84  // enter(clear loop, reset i), exit( 
#define myCCs.add       85  // 
#define myCCs.displace  86  // 
#define myCCs.reScale   87  // 
#define myCCs.reModel   88  // half/double/etc SPEED, swing, quantize ?
#define myCCs.reOrder   89  // reverse, shuffle, invert, 

// real time (for non-MIDI UI ?);
unsigned long T_now_ms;
unsigned long T_then_ms;
unsigned long T_span_ms;


/*
  ##################################################
  MIDI LIBRARY
  ##################################################
*/
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

/*
  ##################################################
  utility Functions
  ##################################################
*/

/*
   ########################
   tempo UO, "MY Q"uarter note
*/
void myQ_advance() {
  iTick++;
  if ( // On beat    // "ON" w/in beat myPPQ
    iTick > -1
    &&
    iTick < blinkPs
  ) {
    uo_myQ_On();
  }
  if (                // "OFF" w/in beat myPPQ
    iTick >= blinkPs
    &&
    iTick < myPPQ
  ) {
    uo_myQ_Off();
  }
  if ( iTick >= myPPQ) { // reset beat w/in myPPQ
    iTick = 0;
  }
}

void uo_myQ_On()         // light ON
{
  digitalWrite(pinLED_seam, HIGH);
}

void uo_myQ_Off()         // light OFF
{
  digitalWrite(pinLED_seam, LOW);
}




/*
   ########################
   LOOP UO, seam
*/
void mySeam_Reset() {
  myLoop.i = 0;
}

void mySeam_advance() {
  myLoop.i ++;
  
  if ( // On beat    // "ON" w/in beat myPPQ
    myLoop.i  > -1
    &&
    myLoop.i  < blinkPs
  ) {
    uo_mySeam_On();
  }
  if (                // "OFF" w/in beat myPPQ
    myLoop.i  >= blinkPs
    &&
    myLoop.i  < myLoop.N
  ) {
    uo_mySeam_Off();
  }
  if ( myLoop.i  >= myLoop.N) { // reset beat w/in myPPQ
    myLoop.i  = 0;
  }
}
void uo_mySeam_On()         // light ON
{
  digitalWrite(pinLED_seam, HIGH);
}

void uo_mySeam_Off()         // light OFF
{
  digitalWrite(pinLED_seam, LOW);
}
/*
   ########################
   LOOP handler
*/

/*
  /////////////////////////////////////////////////
  SETUP
  /////////////////////////////////////////////////
*/
void setup() {
  MIDI.begin();
  //  Set MIDI baud rate:

  // setup and initialize UO
  pinMode(pinLED_q, OUTPUT);
  
  // lights OUT
  myUX_Off();
  uo_mySeam_Off();

}

/*
  ##################################################
  MAIN SCRIPT
  ##################################################
*/
void loop() {

  if (MIDI.read()  )                // Is there a MIDI message incoming ?
  {
    switch (MIDI.getType())         // get the Type of the message we caught, and switch() upon it...
    {

      // handle clock Pulses to advance time
      case midi::Clock:
        
        // UO modluo
        myQ_advance();
        mySeam_advance();

        loopStatus(); // update/handle loop, based on  >>> myLoop.stateCurrent <<<
         
        break;


      // handle Sequencer stage-change to reset "beat UX"
      case midi::Start:
      case midi::Continue:
        // case midi::Stop:

        myHandler_clockReset();
        break;

      /*
         HANDLE CCs
      */
      case midi::ControlChange: // if it's a CC, react to specific myCC #s...
        switch(MIDI.getData1() ){

          case myCCs.reStart :
            myLoop.i = reZero(myLoop.i); // reset to "seam"
          
          case myCCs.proceed :
            myLoop.stateCurrent = "playing";

            break;
            
          case myCCs.halt :
            myLoop.stateCurrent = "stopped";
            break;

          case myCCs.reSeam : 
            // ???
            break;           
      }

      // See the online reference for other message types

      default:
        // nothing happens.
        break;
    }
  }



}
