class IR_Range
{  
private:
  unsigned char mPin;     //Data pin for ir sensor
  float mWeight;          //Smoothing for exp weighted avg. 0-1 lower is smoother
  float mAverage;         //Current average

public:
  IR_Range(){};
  IR_Range(unsigned char pin, float weight)
  {
    init(pin,weight);
  }
  
  void init(unsigned char pin, float weight)
  {
    mPin = pin;
    mAverage=0.0f;
    mWeight=weight;
    pinMode(mPin, INPUT);
  }

  float getStatus()
  {
    int x = analogRead(mPin);
    mAverage = (1 - mWeight) * mAverage + mWeight * x;
    return mAverage;
  }
};
