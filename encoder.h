class Encoder
{
public:
  enum
  {
    NEGATIVE = -1,
    NONE = 0,
    POSITIVE = 1
  };
  
private:
  unsigned char mPin[2];     //Data pins for rotatary encoder
  boolean mEncoderLast;      //Last encoder value

public:
  Encoder(){};
  Encoder(unsigned char pin0, 
          unsigned char pin1)
  {
    init(pin0, pin1);
  }
  
  void init(unsigned char pin0, 
          unsigned char pin1)
  {
    mPin[0] = pin0;
    mPin[1] = pin1;

    mEncoderLast = LOW;
    
    for (int i=0;i<2;++i)
    {
      pinMode(mPin[i], INPUT);
      digitalWrite(mPin[i], HIGH);
    }
    mEncoderLast = digitalRead(mPin[0]);
  }

  signed char getStatus()
  {
    signed char result = NONE;
    
    boolean encoder[2];
    for (int i=0;i<2;++i)
    {
      encoder[i] = digitalRead(mPin[i]);
    }
    
    if (mEncoderLast != encoder[0])
    {
      if (encoder[1] == encoder[0])
      {
        result = POSITIVE;
      }
      else
      {
        result = NEGATIVE;
      }
    }
    mEncoderLast = encoder[0];
    return result;
  }
};
