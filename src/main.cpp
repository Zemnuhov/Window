#define BLYNK_TEMPLATE_ID "TMPLEp3n863j"
#define BLYNK_DEVICE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "bCK2N_Fp3NxZYzK5doE941OsPD6uFRl9"
/*#define BLYNK_PRINT Serial*/
// #define BLYNK_TEMPLATE_ID "TMPLEp3n863j"
// #define BLYNK_TEMPLATE_NAME "Quickstart Template"
// #define BLYNK_AUTH_TOKEN "JG--csQvzfyVCfhtNm1kJQT1HPchOtsB"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleESP32.h>
#include <Arduino.h>
#include "BLE.h"
#include "Window.h"
#include <EEPROM.h>
#include <ArduinoOTA.h>

char auth[] = BLYNK_AUTH_TOKEN;

String ssid = "ZemnhovMob";
String pass = "2405199";
int lastConnectionAttempt = millis();
bool isBlynk = true;

bool isOtaIsInit = false;

BLE ble = BLE();
Window window = Window();

bool firstStart = true;

void bleOpenStateCallback(int state)
{
  window.setPosition(state);
}

void windowStateHasChangedCallback(int state)
{
  Serial.print("state: ");
  Serial.println(state);
  if (state == window.OPEN_STATE)
  {
    ble.setState(true);
  }
  if (state == window.CLOSE_STATE)
  {
    ble.setState(false);
  }
}

void encoderPositionCallback(int position)
{
  ble.setPosition(position);
}

void temperatureCallback(int temperature)
{
  ble.setTemperature(temperature);
}

char *toCharArray(String str)
{
  char *buf = new char[str.length() + 1];
  str.toCharArray(buf, str.length() + 1);
  return buf;
}

void updateNetworkChecker()
{
  String bleSsid = ble.getSsid();
  String blePassword = ble.getPassword();
  if (blePassword != "null")
  {
    pass = blePassword;
    EEPROM.put(0, pass);
    EEPROM.commit();
  }
  if (bleSsid != "null")
  {
    ssid = bleSsid;
    EEPROM.put(50, ssid);
    EEPROM.commit();
  }
  Serial.println(ssid);
  Serial.println(pass);
}

void blynkConnect()
{
  if (ble.isConnect())
  {
    ble.disconnectDevice();
  }
  updateNetworkChecker();
  Serial.println(ssid);
  Serial.println(pass);
  Blynk.begin(auth, toCharArray(ssid), toCharArray(pass));
  // ble.setIP(toCharArray(WiFi.localIP().toString()));
}

BLYNK_WRITE(V0)
{
  u_int8_t value = param.asInt();
  Blynk.virtualWrite(V1, value);
  if (value == 0)
  {
    window.setPosition(0);
  }
  if (value == 1)
  {
    window.setPosition(100);
  }
}

void myTimerEvent()
{
  int8_t temperature = 0;
  if (WiFi.status() == WL_CONNECTED)
  {
    Blynk.virtualWrite(V2, temperature);
  }
  ble.setTemperature(temperature);
}

void blynkWork()
{

  if (WiFi.status() == WL_CONNECTED)
  {
    static long lastIpWrite = millis();
    if (millis() - lastIpWrite > 10000)
    {
      lastIpWrite = millis();
      char *ip = toCharArray(WiFi.localIP().toString());
      Serial.println(ip);
      ble.setIP(ip);
      updateNetworkChecker();
    }
      Blynk.run();
    }
    else
    {
      if (millis() - lastConnectionAttempt > 30000 && ble.isBlynkActive() && !ble.isConnect()) // && !ble.isConnect())
      {
        lastConnectionAttempt = millis();
        blynkConnect();
      }
    }
  }

  void bleListenerBlynkState()
  {
    if (isBlynk != ble.isBlynkActive())
    {
      isBlynk = ble.isBlynkActive();
      if (isBlynk)
      {
        blynkConnect();
      }
      else
      {
        Blynk.disconnect();
      }
    }
  }

  void setup()
  {
    WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    EEPROM.begin(512);
    EEPROM.get(0, pass);
    EEPROM.get(50, ssid);
    Serial.println(ssid);
    Serial.println(pass);
    // WiFi.begin(toCharArray(ssid), toCharArray(pass));
    ArduinoOTA
        .onStart([]()
                 {
		          String type;
		          if (ArduinoOTA.getCommand() == U_FLASH)
			            type = "sketch";
		          else 
			            type = "filesystem";
		          Serial.println("Start updating " + type); })
        .onEnd([]()
               { Serial.println("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
        .onError([](ota_error_t error)
                 {
		        Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
    ArduinoOTA.begin();
    ble.init();
    ble.registerSetOpenStateCallback(bleOpenStateCallback);
    window.init();
    window.registerWindowsStateCallback(windowStateHasChangedCallback);
    window.registerEncoderPositionCallback(encoderPositionCallback);
    window.registerTemperatureCallback(temperatureCallback);
    blynkConnect();
    // window.calibrateEncoder();
  }

  long last = 0;

  void loop()
  {
    if (!ble.isConnect())
    {
      ble.advertising();
    }
    bleListenerBlynkState();
    blynkWork();
    window.work();

    ArduinoOTA.handle();
  }