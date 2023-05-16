#include <Arduino.h>

class Motor
{
private:
    u_int8_t openPin;
    u_int8_t closePin;
    int state = STAND;
public:

    static const int MOVE_OPEN = 1;
    static const int MOVE_CLOSE = 2;
    static const int STAND = 0;

    Motor(u_int8_t _closePin, u_int8_t _openPin){
        closePin = _closePin;
        openPin = _openPin;
    }

    void init(){
        pinMode(closePin, OUTPUT);
        pinMode(openPin, OUTPUT);
        digitalWrite(closePin, LOW);
        digitalWrite(openPin, LOW);
    }

    void open(){
        state = MOVE_OPEN;
        digitalWrite(closePin, LOW);
        digitalWrite(openPin, HIGH);
    }

    void close(){
        state = MOVE_CLOSE;
        digitalWrite(openPin, LOW);
        digitalWrite(closePin, HIGH);
    }

    void stop(){
        state = false;
        digitalWrite(closePin, LOW);
        digitalWrite(openPin, LOW);
    }

    int getState(){
        return state;
    }
    ~Motor(){}
};
