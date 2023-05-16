#include "Arduino.h"
#include "Encoder.h"
#include "NoiseSensor.h"
#include "Motor.h"
#include "Trailer.h"
#include "Settings.h"
#include "TemperatureSensor.h"
#include "Button.h"

class Window
{
private:
    /////////////Pins//////////////
    byte temperatureSensorPin = 18;
    byte encoderFirstPin = 2;
    byte encoderSecondPin = 15;
    byte noiseSensorPin = 19;
    byte motorClosePin = 22;
    byte motorOpenPin = 23;
    byte openTrailerPin = 34;
    byte closeTrailerPin = 35;

    byte buttonClosePin = 33;
    byte buttonOpenPin = 32;
    ///////////////////////////////
    ////////////Objects/////////////
    TemperatureSensor temperatureSensor = TemperatureSensor(temperatureSensorPin);
    Encoder encoder = Encoder(encoderFirstPin, encoderSecondPin);
    NoiseSensor noiseSensor = NoiseSensor(noiseSensorPin);
    Motor motor = Motor(motorClosePin, motorOpenPin);
    Trailer openTrailer = Trailer(openTrailerPin);
    Trailer closeTrailer = Trailer(closeTrailerPin);
    Settings settings = Settings();

    Button closeButton = Button(buttonClosePin);
    Button openButton = Button(buttonOpenPin);
    ////////////////////////////////

    u_int8_t windowState = CLOSE_STATE;
    void (*windowStateHasChangedCallback)(int state);
    void (*encoderPositionCallback)(int position);
    void (*temperatureCallback)(int temperature);

    int requiredPosition = -1000000;

    bool fixState = false;
    int encoderPosition = 0;

    long lastClickToOpenButton = 0;
    u_int8_t buttonOpenClickCounter = 0;
    long lastClickToCloseButton = 0;
    u_int8_t buttonCloseClickCounter = 0;

    long lastTemperatureWrite = 0;

    void temperatureListener()
    {
        if (settings.temperatureSensorIsActive())
        {
            if (temperatureSensor.getTemperature() < settings.getTemperatureSensorThreshold())
            {
                requiredPosition = 0;
                closeWindow();
            }

            if(millis() - lastTemperatureWrite > 10000){
                lastTemperatureWrite = millis();
                temperatureCallback(temperatureSensor.getTemperature());
                Serial.println(temperatureSensor.getTemperature());
            }
            
        }
    }

    void motorProtection(){
        if(motor.getState() == motor.MOVE_OPEN){
            if(openTrailer.isPush()){
                motor.stop();
            }
        }
        if(motor.getState() == motor.MOVE_CLOSE){
            if(closeTrailer.isPush()){
                motor.stop();
            }
        }
    }



    void noiseListener(){
        if(settings.noiseSensorIsActive()){
            if(noiseSensor.getValue() > settings.getNoiseSensorThreshold()){
                requiredPosition = 0;
                closeWindow();
            }
        }
    }

    void openWindow()
    {
        //Serial.println("Open Command");
        if (!openTrailer.isPush())
        {
            //Serial.println("motor.left()");
            motor.open();
        }
    }

    void closeWindow()
    {
        Serial.println("Close Command");
        if (!closeTrailer.isPush())
        {
            Serial.println("motor.right()");
            motor.close();
        }
    }

    void stateListener(){
        if(openTrailer.isPush() && windowState != OPEN_STATE){
            windowState = OPEN_STATE;
            windowStateHasChangedCallback(OPEN_STATE);
            encoderPositionCallback(100);
        }
        if(closeTrailer.isPush() && windowState != CLOSE_STATE){
            windowState = CLOSE_STATE;
            windowStateHasChangedCallback(CLOSE_STATE);
            encoderPositionCallback(0);
        }
    }

    void encoderListener(){
        encoder.work();
        if(encoder.getState() == requiredPosition){
            motor.stop();
        }
        int currPos = encoder.getState();
        if(encoderPosition != currPos){
            encoderPosition = currPos;
            encoderPositionCallback(map(encoderPosition,0, encoder.getMaxState(), 0, 100));
        }
        if(closeTrailer.isPush()){
                encoder.minCalibrate();
        }
        if(openTrailer.isPush()){
            encoder.maxCalibrate();
        }
    }

    void buttonListener(){
        if(openButton.isPush() && millis()-lastClickToOpenButton > 500){
            buttonOpenClickCounter++;
            lastClickToOpenButton = millis();
        }
        if(buttonOpenClickCounter == 1){
            setPosition(100);
        }
        if(!openButton.isPush() && buttonOpenClickCounter >= 2){
            motor.stop();
            buttonOpenClickCounter = 0;
        }

        if(millis() - lastClickToOpenButton > 3000 && buttonOpenClickCounter != 2){
            buttonOpenClickCounter = 0;
        }
        ////////////////////////////////////////////////////////////////////////

        if(closeButton.isPush() && millis()-lastClickToCloseButton > 500){
            buttonCloseClickCounter++;
            lastClickToCloseButton = millis();
        }
        if(buttonCloseClickCounter == 1){
            setPosition(0);
        }
        if(!closeButton.isPush() && buttonCloseClickCounter >= 2){
            motor.stop();
            buttonCloseClickCounter = 0;
        }

        if(millis() - lastClickToCloseButton > 3000 && buttonCloseClickCounter != 2){
            buttonCloseClickCounter = 0;
        }

    }

public:

    static const u_int8_t OPEN_STATE = 1;
    static const u_int8_t CLOSE_STATE = 0;


    Window() {}



    void registerWindowsStateCallback(void (*callback)(int state)){
        windowStateHasChangedCallback = callback;
    }

    void registerEncoderPositionCallback(void (*callback)(int position)){
        encoderPositionCallback = callback;
    }

    void registerTemperatureCallback(void (*callback)(int temperature)){
        temperatureCallback = callback;
    }

    void init()
    {
        temperatureSensor.init();
        encoder.init();
        motor.init();
        openTrailer.init();
        closeTrailer.init();
    }

    void work()
    {
        temperatureListener();
        motorProtection();
        encoderListener();
        noiseListener();
        stateListener();
        buttonListener();
    }

    void setPosition(int position){
        if(position == 0){
            requiredPosition = -1000;
        }else if(position == 100){
            requiredPosition = 1000;
        }else{
            requiredPosition = map(position, 0, 100, encoder.getMinState(), encoder.getMaxState());
        }
        Serial.print("requiredPosition:");
        Serial.println(requiredPosition);
        if(requiredPosition > encoder.getState()){
            openWindow();
        }
        if(requiredPosition < encoder.getState()){
            closeWindow();
        }
    }

    void calibrateEncoder(){
        closeWindow();
        while (closeTrailer.isPush() == false){work();}
        Serial.println("motor.stop()");
        motor.stop();
        encoder.minCalibrate();
        openWindow();
        while (openTrailer.isPush() == false){work();}
        Serial.println("motor.stop()");
        motor.stop();
        encoder.maxCalibrate();
        closeWindow();
    }

    ~Window() {}
};
