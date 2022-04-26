#include "encoder.h"
#include "button.h"
#include "ir_range.h"

#include <Adafruit_NeoPixel.h>  //Someone else did the heavy lifting.  Say thanks!
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "rain.h"
#include "solid.h"

//Times are in miliseconds.  60000ms to the minute.
const unsigned long gTimeToCooldown = 1000l * 3l;        //3 sec
const unsigned long gTimeToSave = 1000l * 30l;           //30 sec
const unsigned long gTimeToIdle = 1000l * 60l * 1l;       //1 min
const unsigned long gTimeToOff  = 1000l * 60l * 60l * 2l; //2 hr

const short PIN_LED = 6;  //Pin connecting the IN on the LED strip to the CPU board.
const short NUM_LEDS = 30; //Num LEDS in our array.

Adafruit_NeoPixel H_LEDS = Adafruit_NeoPixel(NUM_LEDS, PIN_LED,NEO_GRBW + NEO_KHZ800);

//Initialize input hardware
Button gButton(12);                 //Init encoder button as pin
Encoder gEncoder(4,3);              //Init encoder dial on pins
IR_Range gRanger(A1,0.2f);          //Init range finder on pin with wgt

//Initialize pattern classes
Rain rain;
Solid solid(0,1);

enum      //Operational modes of the state machine.
{
  MODE_OFF = 0,
  MODE_WHITE = 1,
  MODE_PATTERN = 2,
  MODE_SAVING = 128 
};

enum      //Behavior of the rotary encoder
{
  RACT_NONE = 0, 
  RACT_BRIGHT
};

//Pollute the global namespace!
unsigned char gMode = MODE_OFF;       //Start off Off.
unsigned char gRact = RACT_BRIGHT;    //What the rotary encoder does right now.
signed int gMaxBrightness = 255;      //Maximum brightness.
signed char gFadeBrightness = 255;    //Fade out brightness. 
bool gNeedsSave = false;
unsigned long gStartTime = 0;         //Used when we've got a timer happening.
float gLastRange=0.0f;                //Used for idle out
bool gMotionActive = true;

unsigned char clamp(signed int val)  //make int range 0-255
{
  unsigned char result=val;
  if (val > 255) result = 255;
  else if (val < 0) result = 0;
  return result;
}

void turnOff()      //go to sleeep 
{
  Serial.println("Off");
  //TODO doSave();  //Save if needed.
  COLOR c;
  c.l = 0;
  solid.setColor(c);
  gMode = MODE_OFF; 
  gMotionActive = false;  //Make handwave not work for a bit.
}

void turnOn() //Bump up to white light.
{
  COLOR c;
  c.l = 0xFFFFFFFF;
  solid.setColor(c);
  gMode= MODE_WHITE;
}

void getButtonStatus() {
  // Get button event and act accordingly
  int b = gButton.getStatus();
  if (b != Button::UNDEF)   //got some button action
  { 
    if (b == Button::CLICK) 
    {
      if (gMode == MODE_OFF || gMode == MODE_PATTERN)
      { //Switch to white if off or in pattern mode.
        turnOn();
      }
      else if (gMode == MODE_WHITE)
      { //Switch to pattern mode.
        gMode = MODE_PATTERN;
      }
      gNeedsSave = true;
      gStartTime = millis();
    }
    else if (b == Button::LONGHOLD)
    { //Turn off
      //TODO doSave();  //Save if needed.
      turnOff();
      gMotionActive=false;
    }
    /*
    else if (b == Button::DOUBLECLICK) 
      Serial.println("Double Click");
    else if (b == Button::SHORTHOLDDETECT)
      Serial.println("1 sec hold detect");
    else if (b == Button::SHORTHOLD) 
      Serial.println("1 sec hold release");
    else if (b == Button::LONGHOLD) 
      Serial.println("3 sec hold");
    */
  }
}

void getEncoderStatus() { //Rotary encoder turns left and right
  signed char x=gEncoder.getStatus(); //returns -1, 0, or 1

  if (gRact == RACT_BRIGHT) //Encoder adjusts the brightness.
  {
    gMaxBrightness += x*25;
    gMaxBrightness = clamp(gMaxBrightness);
  }
  /*
  if (x==Encoder::NEGATIVE)
    Serial.println("Left");
  else if (x==Encoder::POSITIVE)
    Serial.println("Right");
    */
}

void getRange()  //Handle the range sensor
{ 
  float range = gRanger.getStatus();
  if (gMotionActive)
  {
    if ((int)abs(gLastRange - range) > 40) //Movement detected.
    {
      gStartTime = millis();
      if (gMode == MODE_PATTERN) //Move back to white light.
      {
        turnOn();
      }
      else if (gMode == MODE_OFF)
      {
        gMode = MODE_PATTERN;     //Wakey wakey
      }
      gMotionActive = false;
    }
  }
  gLastRange = range;
}

void trimBrightness()
{
  unsigned char brightness=gMaxBrightness;
  //if (gFadeBrightness < gMaxBrightness)
  //    brightness = gFadeBrightness;

  for (int p=0;p<NUM_LEDS; ++p)  //Loop through pixels.
  {
    COLOR pc;
    pc.l = H_LEDS.getPixelColor(p);

    for (int c=0;c<4;++c)  //Loop through RBGW sub-pixels of each.
    {
      if (pc.c[c] > brightness) {
        pc.c[c] = gMaxBrightness;
      }
    }
    H_LEDS.setPixelColor(p,pc.l);
  }
}

void doTimers()
{
  unsigned long now = millis();
  unsigned long elapsed = now - gStartTime;
  
  if ( gTimeToCooldown < elapsed )    //Re-activate movement detect
  {
    gMotionActive=true;
  }
   
  if (gNeedsSave && gTimeToSave < elapsed) //TODO from-save time bad.
  {
    //TODO save to memory
    gNeedsSave = false;
  }
  if (gMode == MODE_WHITE)    //When in white light mode
  {
    if (gTimeToIdle < elapsed)  //Idle to pattern.
    {
      gMode = MODE_PATTERN;
    }
  }
  if (gTimeToOff < elapsed and gMode != MODE_OFF) //black out
  {
    turnOff();
  }
}

void doLighting()
{
  if (gMode == MODE_WHITE || gMode == MODE_OFF)
  {
    solid.loopStep(); 
  }
  else
  {
    rain.loopStep();
  }
  trimBrightness();
  H_LEDS.show();
}

void setup() {
  Serial.begin(19200);
  H_LEDS.begin(); //Initialize communication with LED array.
  H_LEDS.show(); //No values are set so this should be black.
  gMotionActive=false; //Allow motion input time to get average
  rain.mShiftOdds = 200;
}

void loop() {
  getEncoderStatus();
  getButtonStatus();
  getRange();
  doTimers();
  doLighting();
}
