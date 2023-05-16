#include <Arduino.h>

class Led
{
private:
    byte _pin;
public:
    Led(byte pin){
        _pin = pin;
    };

    void init(){
        pinMode(_pin, OUTPUT);
    }

    void high(){
        digitalWrite(_pin, HIGH);
    }

    void low(){
        digitalWrite(_pin, LOW);
    }

    ~Led(){};
};

