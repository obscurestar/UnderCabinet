//=================================================
//  MULTI-CLICK:  One Button, Multiple Events
class Button
{
public:
  enum {
    UNDEF = 0,
    CLICK,
    DOUBLECLICK,
    SHORTHOLDDETECT,
    SHORTHOLD,
    LONGHOLD
  };

private:
  // Button timing variables
  unsigned int mDebounce;         // ms debounce period to prevent flickering when pressing or releasing the button
  unsigned int mDCgap;            // Max ms between clicks for a double click event
  unsigned int mHoldTime;         // ms hold period: how long to wait for press+hold event
  unsigned int mLongHoldTime;     // ms long hold period: how long to wait for press+hold event

  // Button variables
  boolean mButtonVal;       // Value read from button
  boolean mButtonLast;      // Buffered value of the button's previous state
  boolean mDCwaiting;     // Whether we're waiting for a double click (down)
  boolean mDConUp;        // Whether to register a double click on next release, or whether to wait and click
  boolean mSingleOK;        // Whether it's OK to do a single click
  long mDownTime;           // Time the button was pressed down
  long mUpTime;             // Time the button was released
  boolean mIgnoreUp;      // Whether to ignore the button release because the click+hold was triggered
  boolean mWaitForUp;         // When held, whether to wait for the up event
  boolean mHoldEventPast;     // Whether or not the hold event happened already
  boolean mLongHoldEventPast; // Whether or not the long hold event happened already
  unsigned char mButtonPin;                 // Pin the button is connected on.

public:
  Button(){}
  Button(unsigned char button_pin, bool pullup=true)
  {
    init(button_pin, pullup);
  }

  void init(unsigned char button_pin, bool pullup)
  {
    mDebounce = 20;
    mDCgap = 250;
    mHoldTime = 1000;
    mLongHoldTime = 3000;
    mButtonVal = HIGH;
    mButtonLast = HIGH;
    mDCwaiting = false;
    mDConUp = false;
    mSingleOK = true;
    mDownTime = -1;
    mUpTime = -1;
    mIgnoreUp = false;
    mWaitForUp = false;
    mHoldEventPast = false;
    mLongHoldEventPast = false;

    mButtonPin = button_pin;
    pinMode(mButtonPin, INPUT);     //Configure for input.
              digitalWrite(mButtonPin, pullup );  //Use internal pullup.
  }

  int getStatus() 
  {    
    int event = UNDEF;
    mButtonVal = digitalRead(mButtonPin);
    // Button pressed down
 
    if (mButtonVal == LOW && mButtonLast == HIGH && (millis() - mUpTime) > mDebounce)
    {
      mDownTime = millis();
      mIgnoreUp = false;
      mWaitForUp = false;
      mSingleOK = true;
      mHoldEventPast = false;
      mLongHoldEventPast = false;
      if ((millis()-mUpTime) < mDCgap && mDConUp == false && mDCwaiting == true)  mDConUp = true;
      else  mDConUp = false;
      mDCwaiting = false;
    }
    // Button released
    else if (mButtonVal == HIGH && mButtonLast == LOW && (millis() - mDownTime) > mDebounce)
    {        
      if (mIgnoreUp)
      {
        if (mHoldEventPast && !mLongHoldEventPast)
        {
          event=SHORTHOLD;
        }
      }
      else
      {
        mUpTime = millis();
        if (mDConUp == false) mDCwaiting = true;
        else
        {
          event = DOUBLECLICK;
          mDConUp = false;
          mDCwaiting = false;
          mSingleOK = false;
        }
      }
    }
    // Test for normal click event: mDCgap expired
    if ( mButtonVal == HIGH && (millis()-mUpTime) >= mDCgap && mDCwaiting == true && mDConUp == false && mSingleOK == true && event != 2)
    {
      event = CLICK;
      mDCwaiting = false;
    }
    // Test for hold
    if (mButtonVal == LOW && (millis() - mDownTime) >= mHoldTime) {
      // Trigger "normal" hold
      if (! mHoldEventPast)
      {
        event = SHORTHOLDDETECT;
        mWaitForUp = true;
        mIgnoreUp = true;
        mDConUp = false;
        mDCwaiting = false;
        //mDownTime = millis();
        mHoldEventPast = true;
      }
      // Trigger "long" hold
      if ((millis() - mDownTime) >= mLongHoldTime)
      {
        if (! mLongHoldEventPast)
        {
          event = LONGHOLD;
          mLongHoldEventPast = true;
        }
      }
    }
    mButtonLast = mButtonVal;
    return event;
  }
};
