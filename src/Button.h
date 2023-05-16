#include "Arduino.h"

class Button{
    private:
        u_int8_t _buttonPin;

    public:
    Button(u_int8_t pin){
        _buttonPin = pin;
        pinMode(_buttonPin, INPUT);
    }

    bool isPush(){
        return !digitalRead(_buttonPin);
    }


    ~Button(){}
};