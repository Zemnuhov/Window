#include <Arduino.h>

class Trailer
{
private:
    byte pin;    
    bool previous_state = false;
public:
    
    Trailer(u_int8_t _pin){
        pin = _pin;
    }

    void init(){
        pinMode(pin, INPUT);
    }

    bool isPush(){
        bool state = digitalRead(pin);
        if(state){
            return false;
        }else{
            return true;
        }
        if(state != previous_state){
            Serial.println(state);
            previous_state = state;
        }
    }

    ~Trailer(){}
};
