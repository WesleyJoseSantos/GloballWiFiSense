#ifndef GLOBALLWIFISENSE_H
#define GLOBALLWIFISENSE_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

// #define SERIAL_DEBUG Serial

#define PIN_PROV 4
#define PIN_INPUT 5

#define WIFI_SSID "cabo canaveral"
#define WIFI_PASS "05151924"

#define MQTT_HOST IPAddress(192, 168, 0, 101)
#define MQTT_PORT 1883
#define MQTT_QOS 2

#define MQTT_TOPIC_SENSE_TO_BROKER "Sense To Broker/"

#define MSG_SEPARATOR (String)"/"
#define MSG_GPIO_ON "HIGH"
#define MSG_GPIO_OFF "LOW"

#define DEEP_SLEEP_TIME 1e6 //1 segundo

enum RtcAddressMap{
    RTC_SENSE_STATE
};

enum SenseState{
    ST_SENSE_GPIO_ON,
    ST_SENSE_GPIO_OFF
};

class WiFiSense
{
private:
    WiFiEventHandler wifiConnectHandler;
    WiFiEventHandler wifiDisconnectHandler;
    uint32_t state;
    uint32_t previousState;
    void setupPins();
    bool isNewState();
    void checkState();
public:
    WiFiSense();
    void start();
    void updateMqtt();
    void sendMqtt(String msg);
};

void connectToWifi();
void onWifiConnect(const WiFiEventStationModeGotIP& event);
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);
void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttPublish(uint16_t packetId);

extern WiFiSense wifiSense;

#endif