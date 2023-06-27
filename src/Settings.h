class Settings
{
private:
    bool noiseSensorState = true;
    u_int16_t noiseSensorThresholdValue = 2000;

    bool temperatureSensorState = true;
    u_int8_t temperatureSensorThresholdValue = 100;
public:
    Settings(){}
    bool noiseSensorIsActive(){
        return noiseSensorState;
    }

    u_int8_t getNoiseSensorThreshold(){
        return noiseSensorThresholdValue;
    }

    bool temperatureSensorIsActive(){
        return temperatureSensorState;
    }

    u_int8_t getTemperatureSensorThreshold(){
        return temperatureSensorThresholdValue;
    }

    void setNoiseSensorState(bool state){
        noiseSensorState = state;
    }

    void setNoiseSensorThreshold(int value){
        noiseSensorThresholdValue = value;
    }

    void setTemperatureSensorState(bool state){
        temperatureSensorState = state;
    }

    void setTemperatureSensorThreshold(int value){
        temperatureSensorThresholdValue = value;
    }
    ~Settings(){}
};
