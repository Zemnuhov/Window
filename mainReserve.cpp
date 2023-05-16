#define BLYNK_TEMPLATE_ID "TMPLEp3n863j"
#define BLYNK_DEVICE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "bCK2N_Fp3NxZYzK5doE941OsPD6uFRl9"

#define BLYNK_PRINT Serial

#define SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fb"
#define temperatureCharacteristicUUID "0000ffe1-0000-1000-8000-00805f9b34fb"
#define configurationCharacteristicUUID "0000ffe2-0000-1000-8000-00805f9b34fb"
#define ssidCharacteristicUUID "0000ffe3-0000-1000-8000-00805f9b34fb"
#define passwordCharacteristicUUID "0000ffe4-0000-1000-8000-00805f9b34fb"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Arduino.h>
#include "TemperatureSensor.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

char auth[] = BLYNK_AUTH_TOKEN;

String ssid = "ZemnhovMob";
String pass = "24051998";
int lastConnectionAttempt = 0;

BlynkTimer timer;
TemperatureSensor temperatureSensor = TemperatureSensor(14);
BLEServer *pServer;
BLECharacteristic *temperatureCharacteristic;
BLECharacteristic *configurationCharacteristic;
BLECharacteristic *ssidCharacteristic;
BLECharacteristic *passwordCharacteristic;

bool advertising = false;
bool deviceConnected = false;

char *toCharArray(String str){
  char *buf=new char[str.length()+1];
  str.toCharArray(buf,str.length()+1);
  return buf;
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      advertising = false;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class ConfigurationCallbaks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
      uint8_t value = *configurationCharacteristic->getData();
      switch (value)
      {
      case 10:
        Blynk.disconnect();
        break;
      case 11:
        pServer->disconnect(pServer->getConnId());
        Serial.println(ssid);
        Serial.println(pass);
        Blynk.begin(auth, toCharArray(ssid), toCharArray(pass));
        break;
      default:
        break;
      }
  }
};

class SsidCallbaks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      ssid = value.c_str();
  }
};

class PasswordCallbaks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      pass = value.c_str();
  }
};

void bleAdvertising(){
  if(!deviceConnected && !advertising){
    pServer->getAdvertising()->start();
    advertising = true;
  }
}

BLYNK_WRITE(V0)
{
  int value = param.asInt();
  Blynk.virtualWrite(V1, value);
}

BLYNK_CONNECTED()
{
  //Зажечь счетодиод LED
}

void myTimerEvent()
{
  int temperature = temperatureSensor.getTemperature();
  if(WiFi.status() == WL_CONNECTED){
    Blynk.virtualWrite(V2, temperature);
  }
  if(deviceConnected){
    temperatureCharacteristic->setValue(temperature);
    temperatureCharacteristic->notify();
  }
}

void blynkWork(){
  if (WiFi.status() == WL_CONNECTED){
    Blynk.run();
    timer.run();
  }
  if (millis() - lastConnectionAttempt > 30000 && WiFi.status() != WL_CONNECTED) {
    lastConnectionAttempt = millis();
    pServer->disconnect(pServer->getConnId());
    Serial.println(ssid);
    Serial.println(pass);
    Blynk.begin(auth, toCharArray(ssid), toCharArray(pass));
  }
}

void setup()
{
  Serial.begin(115200);
  BLEDevice::init("Window");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  configurationCharacteristic = pService->createCharacteristic(configurationCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
  ssidCharacteristic = pService->createCharacteristic(ssidCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
  passwordCharacteristic = pService->createCharacteristic(passwordCharacteristicUUID, BLECharacteristic::PROPERTY_WRITE);
  configurationCharacteristic->setCallbacks(new ConfigurationCallbaks());
  ssidCharacteristic->setCallbacks(new SsidCallbaks());
  passwordCharacteristic->setCallbacks(new PasswordCallbaks());
  pService->start();
  bleAdvertising();
  temperatureSensor.init();
  //Blynk.begin(auth, toCharArray(ssid), toCharArray(pass));
  timer.setInterval(1000L, myTimerEvent);
}

void loop()
{
  if(!deviceConnected){
    bleAdvertising();
  }
  blynkWork();
}