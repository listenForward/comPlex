/*
   (cc) Patrick Richardson, 2016,
   sketch to blink if/when a MIDI IN port (on uArt RX pin) receives a specific MIDI message of type
     NoteOn
*/


/*
  ##################################################
  CONFIG
  ##################################################
*/
// UI
#define pinLED      13
#define blinkDelay  10  // mSec


// MIDI clock tick segmentation
#define myPPQ   24
#define blinkPs 6     // hold for 8 clockPulses after ON, then turn OFF...
int iTick     = 0;

// time
unsigned long T_now_ms;
unsigned long T_then_ms;
unsigned long T_span_ms;

int noteVelocity;

/*
  ##################################################
  MIDI LIBRARY
  ##################################################
*/
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

/*
  ##################################################
  utility Functionå
  ##################################################
*/


void myHandler_clockReset(){
  iTick = 0; 
}

void myHandler_clockAdvance(){
  iTick++;

  if( // On beat    // "ON" w/in beat myPPQ
      iTick > -1 
      && 
      iTick < blinkPs
    ){
    myUX_On();
  }

  if(                 // "OFF" w/in beat myPPQ
      iTick >= blinkPs 
      && 
      iTick < myPPQ
    ){
    myUX_Off();
  }

  if( iTick >= myPPQ){   // reset beat w/in myPPQ
    iTick = 0;
  }

}


void myUX_On()         // light ON
{
  digitalWrite(pinLED, HIGH);  
}


void myUX_Off()         // light OFF
{
  digitalWrite(pinLED, LOW);
  
}



/*
  /////////////////////////////////////////////////
  SETUP
  /////////////////////////////////////////////////
*/
void setup() {  
  MIDI.begin();
  //  Set MIDI baud rate:

  // setup and initialize UO
  pinMode(pinLED, OUTPUT);
  myUX_Off();
  
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
        myHandler_clockAdvance();
        break;  


      // handle Sequencer stage-change to reset "beat UX"
      case midi::Start:
      case midi::Continue:
      // case midi::Stop:
        
        myHandler_clockReset();
        break;  
        
      // See the online reference for other message types

      default:
        // nothing happens.
      break;
    }
  }



}
