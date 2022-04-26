#ifndef SOLID_H
#define SOLID_H
#include <Adafruit_NeoPixel.h>  //Someone else did the heavy lifting.  Say thanks!
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "color.h"

extern const short NUM_LEDS; //Let's address this in V3!
extern Adafruit_NeoPixel H_LEDS;

class Solid
{
public:
  Solid()
  {
    mColor.l=0;   //Default to off
    mTransitionRate=0;  //Instant change
  };

  Solid(long color, int rate)
  {
    mColor.l = color;
    mTransitionRate = rate;
  }
  
  void setColor(COLOR c)
  {
    mColor.l = c.l;
  }
  
  void loopStep()
  {
    for (int p=0;p<NUM_LEDS; ++p)  //Loop through pixels.
    {
      COLOR pc;
      pc.l = H_LEDS.getPixelColor(p);

      if (mColor.l != pc.l)   //Colors don't match.
      {
        for (int c=0;c<4;++c)  //Loop through RBGW sub-pixels of each.
        {
          if (mColor.c[c] != pc.c[c])
          {
            signed char dir = 1;
            if (pc.c[c] > mColor.c[c]) 
            {
              dir = -1;
            }
            if (!random(mTransitionRate))
            {
              pc.c[c]+=dir; //Wander slowly towards 0.
            }
          }
        }
        H_LEDS.setPixelColor(p,pc.l);
      }
    }
  }
  
private:
  COLOR mColor;
private:
  int mTransitionRate;    //Random chance to step Larger is slower
};

#endif //SOLID_H
