#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTTYPE DHT22 

class TemperatureSensor
{
private:
    u_int8_t _pin = 0;
    DHT sensor = DHT(_pin, DHTTYPE);
public:
    TemperatureSensor(u_int8_t pin){
        _pin = pin;
        sensor = DHT(_pin, DHTTYPE);
    }

    void init(){
        sensor.begin();
    }

    float getTemperature(){
        return sensor.readTemperature();
    }

    float getHumidity(){
        return sensor.readHumidity();
    }

    ~TemperatureSensor(){};
};

