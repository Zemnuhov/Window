#include <Arduino.h>

const int NUM_READ = 10;

class NoiseSensor
{
private:
    byte pin;
    int t = 0;
    int vals[NUM_READ];
    int average = 0;
    int noiseValue = 0;

public:
    NoiseSensor(byte _pin)
    {
        pin = _pin;
    }

    void work()
    {
        int value = analogRead(pin);
        if (value < 1800)
        {
            value = 1800 + (1800 - value);
        }

        if (++t >= NUM_READ)
            t = 0;          // перемотка t
        average -= vals[t]; // вычитаем старое
        average += value;   // прибавляем новое
        vals[t] = value;    // запоминаем в массив
        noiseValue = ((float)average / NUM_READ);
    }

    int getValue(){
        return noiseValue;
    }
    ~NoiseSensor() {}
};
