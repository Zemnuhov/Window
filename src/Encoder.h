#include "Arduino.h"

class Encoder
{
private:
    int state = 0;
    byte encoderBuffer[2];
    u_int8_t firstPin;
    u_int8_t secondPin;
    u_int32_t timer = 0;
    u_int8_t minPosition = 0;
    u_int8_t maxPosition = 38;
    u_int8_t READ_DELAY = 2;
    bool stateALast = false;

public:
    Encoder(u_int8_t _firstPin, u_int8_t _secondPin)
    {
        firstPin = _firstPin;
        secondPin = _secondPin;
    }

    void init()
    {
        pinMode(firstPin, INPUT_PULLUP);
        pinMode(secondPin, INPUT_PULLUP);
    }

    int getMaxState()
    {
        return maxPosition;
    }

    int getMinState()
    {
        return minPosition;
    }

    void minCalibrate()
    {
        state = 0;
    }

    void maxCalibrate()
    {
        maxPosition = state;
    }

    int getState()
    {
        return state;
    }

    void work()
    {
        bool stateA = digitalRead(firstPin);
        bool stateB = digitalRead(secondPin);
        if (stateA != stateALast)
        {
            stateALast = stateA;
            if (stateA != false || stateB != false)
                if (stateA != stateB)
                {
                    state--;
                }
                else
                {
                    state++;
                }
            Serial.print("Encoder:");
            Serial.println(state);
        }
    }
    ~Encoder() {}
};

// void work(){
//     if(timer+5 > millis()) return;
//     encoderBuffer[0] = encoderBuffer[1];
//     encoderBuffer[1] = digitalRead(firstPin);
//     if(encoderBuffer[0] != encoderBuffer[1]){
//         state += digitalRead(secondPin) == encoderBuffer[1] ? 1 : -1;
//         Serial.print("Encoder:");
//         Serial.println(state);
//     }
//     timer = millis();
// }
