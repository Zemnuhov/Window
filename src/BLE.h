#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID "a37530b0-1a89-4a35-b52e-9f1bd5fd05d7"
#define configurationCharacteristicUUID "a37530b1-1a89-4a35-b52e-9f1bd5fd05d7"
#define ssidCharacteristicUUID "a37530b2-1a89-4a35-b52e-9f1bd5fd05d7"
#define passwordCharacteristicUUID "a37530b3-1a89-4a35-b52e-9f1bd5fd05d7"
#define ipCharacteristicUUID "a37530b4-1a89-4a35-b52e-9f1bd5fd05d7"

#define STATE_SERVICE_UUID "6bd5ad50-0624-4c0a-9472-2b016cfe3570"
#define temperatureCharacteristicUUID "6bd5ad51-0624-4c0a-9472-2b016cfe3570"
#define switchCharacteristicUUID "6bd5ad52-0624-4c0a-9472-2b016cfe3570"
#define encoderPositionCharacteristicUUID "6bd5ad53-0624-4c0a-9472-2b016cfe3570"

#define COMMAND_SERVICE_UUID "011cf590-6f83-42a7-9799-98536a472ac0"
#define openStateCommandCharacteristicUUID "011cf591-6f83-42a7-9799-98536a472ac0"

BLEServer *pServer;

BLEService *baseService;
BLECharacteristic *configurationCharacteristic;
BLECharacteristic *ssidCharacteristic;
BLECharacteristic *passwordCharacteristic;
BLECharacteristic *ipCharacteristic;

BLEService *stateService;
BLECharacteristic *temperatureCharacteristic;
BLECharacteristic *switchCharacteristic;
BLECharacteristic *encoderPositionCharacteristic;

BLEService *commandService;
BLECharacteristic *openStateCommandCharacteristic;

bool isAdvertising = false;
bool deviceConnected = false;

String _ssid = "null";
String _pass = "null";

bool blynkIsActive = true;

void (*setOpenStateCallback)(int state);

class BLE
{
private:
    class MyServerCallbacks : public BLEServerCallbacks
    {
        void onConnect(BLEServer *pServer)
        {
            deviceConnected = true;
            isAdvertising = false;
            Serial.println("Connect");
        };

        void onDisconnect(BLEServer *pServer)
        {
            deviceConnected = false;
        }
    };

    class ConfigurationCallbaks : public BLECharacteristicCallbacks
    {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
            uint8_t value = *configurationCharacteristic->getData();
            switch (value)
            {
            case 10:
                blynkIsActive = false;
                Serial.println("BLE:10");
                break;
            case 11:
                blynkIsActive = true;
                Serial.println("BLE:11");
                break;
            default:
                break;
            }
        }
    };

    class SsidCallbaks : public BLECharacteristicCallbacks
    {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
            std::string value = pCharacteristic->getValue();
            _ssid = value.c_str();
            Serial.print("SsidCallbaks: ");
            Serial.println(_ssid);
        }
    };

    class PasswordCallbaks : public BLECharacteristicCallbacks
    {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
            std::string value = pCharacteristic->getValue();
            _pass = value.c_str();
            Serial.print("PasswordCallbaks: ");
            Serial.println(_pass);
        }
    };

    class OpenStateCommandCallbaks : public BLECharacteristicCallbacks
    {
        void onWrite(BLECharacteristic *pCharacteristic)
        {
            u_int8_t value = *openStateCommandCharacteristic->getData();
            Serial.print("OpenStateCommandCallbaks: ");
            Serial.println(value);
            if(value == 0){
                setOpenStateCallback(0);
            }else{
                setOpenStateCallback(value);
            }
        }
    };

public:

    BLE() {}

    void registerSetOpenStateCallback(void (*callback)(int state)){
        setOpenStateCallback = callback;
    }

    void bleBaseServiceInit(){
        baseService = pServer->createService(SERVICE_UUID);
        configurationCharacteristic = baseService->createCharacteristic(configurationCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
        configurationCharacteristic->setCallbacks(new ConfigurationCallbaks());

        ssidCharacteristic = baseService->createCharacteristic(ssidCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
        ssidCharacteristic->setCallbacks(new SsidCallbaks());

        passwordCharacteristic = baseService->createCharacteristic(passwordCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
        passwordCharacteristic->setCallbacks(new PasswordCallbaks());

        baseService->start();
    }

    void bleStateServiceInit(){
        stateService = pServer->createService(STATE_SERVICE_UUID);
        temperatureCharacteristic = stateService->createCharacteristic(temperatureCharacteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);
        temperatureCharacteristic->addDescriptor(new BLE2902());
        switchCharacteristic = stateService->createCharacteristic(switchCharacteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);
        switchCharacteristic->addDescriptor(new BLE2902());
        encoderPositionCharacteristic = stateService->createCharacteristic(encoderPositionCharacteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);
        encoderPositionCharacteristic->addDescriptor(new BLE2902());
        stateService->start();
    }

    void bleCommandServiceInit(){
        commandService = pServer->createService(COMMAND_SERVICE_UUID);
        openStateCommandCharacteristic = commandService->createCharacteristic(openStateCommandCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
        openStateCommandCharacteristic->setCallbacks(new OpenStateCommandCallbaks());
        openStateCommandCharacteristic->addDescriptor(new BLE2902());
        ipCharacteristic = commandService->createCharacteristic(ipCharacteristicUUID, BLECharacteristic::PROPERTY_NOTIFY|BLECharacteristic::PROPERTY_READ);
        ipCharacteristic->addDescriptor(new BLE2902());
        commandService->start();
    }

    void init()
    {
        BLEDevice::init("Window");
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());

        bleBaseServiceInit();
        bleStateServiceInit();
        bleCommandServiceInit();
        
        advertising();
    }

    void advertising()
    {
        if (!deviceConnected && !isAdvertising)
        {
            pServer->getAdvertising()->start();
            isAdvertising = true;
        }
    }

    String getSsid()
    {
        return _ssid;
    }

    String getPassword()
    {
        return _pass;
    }

    void setTemperature(int value)
    {
            temperatureCharacteristic->setValue(value);
            temperatureCharacteristic->notify();
    }

    void setIP(char* ip)
    {

            ipCharacteristic->setValue(ip);
            ipCharacteristic->notify();
    
        
    }

    void setState(bool state){
        Serial.print("Write state: ");
        Serial.println(state);
        int value;
        if(state){
            value = 1;
        }else{
            value = 0;
        }
        switchCharacteristic->setValue(value);
        switchCharacteristic->notify();
    }

    void setPosition(int position){
        Serial.print("Write Position: ");
        Serial.println(position);
        encoderPositionCharacteristic->setValue(position);
        encoderPositionCharacteristic->notify();
    }

    bool isConnect()
    {
        return deviceConnected;
    }

    bool isBlynkActive(){
        return blynkIsActive;
    }

    void disconnectDevice()
    {
        pServer->disconnect(pServer->getConnId());
    }

    ~BLE() {}
};
