#include "WiFiSense.h"

AsyncMqttClient mqtt;
Ticker wifiReconnectTimer;
Ticker mqttReconnectTimer;

WiFiSense wifiSense;

WiFiSense::WiFiSense()
{

}

void WiFiSense::setupPins()
{
  pinMode(PIN_PROV, INPUT_PULLUP);
  pinMode(PIN_INPUT, INPUT_PULLUP);
}

void WiFiSense::start()
{
    #ifdef SERIAL_DEBUG
    SERIAL_DEBUG.begin(74880);
    SERIAL_DEBUG.println();
    SERIAL_DEBUG.println();
    SERIAL_DEBUG.println("START!");
    #endif

    setupPins();

    if (!isNewState()){
      #ifdef SERIAL_DEBUG
      SERIAL_DEBUG.println("Going to Sleep");
      #endif
      ESP.deepSleep(DEEP_SLEEP_TIME);
    }

    ESP.rtcUserMemoryWrite(RTC_SENSE_STATE, &state, sizeof(state));

    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    mqtt.onConnect(onMqttConnect);
    mqtt.onDisconnect(onMqttDisconnect);
    mqtt.onPublish(onMqttPublish);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();
}

bool WiFiSense::isNewState()
{
  ESP.rtcUserMemoryRead(RTC_SENSE_STATE, &previousState, sizeof(previousState));
  checkState();
  bool ret = (state != previousState);
  return(ret);
}

void WiFiSense::updateMqtt()
{
  switch (state)
  {
  case ST_SENSE_GPIO_ON:
    #ifdef SERIAL_DEBUG
    SERIAL_DEBUG.println("MSG_DEVICE_STARTED");
    #endif
    sendMqtt(MSG_GPIO_ON);
    break;
  case ST_SENSE_GPIO_OFF:
    #ifdef SERIAL_DEBUG
    SERIAL_DEBUG.println("MSG_DEVICE_STARTED");
    #endif
    sendMqtt(MSG_GPIO_OFF);
    break;
  default:
    break;
  }
}

void WiFiSense::checkState()
{
  if (!digitalRead(PIN_INPUT))
  {
    state = ST_SENSE_GPIO_ON;
    #ifdef SERIAL_DEBUG
    SERIAL_DEBUG.println("ST_SENSE_GPIO_ON");
    #endif
    return;
  }
  else
  {
    state = ST_SENSE_GPIO_OFF;
    #ifdef SERIAL_DEBUG
    SERIAL_DEBUG.println("ST_SENSE_GPIO_OFF");
    #endif
    return;
  }
}

void WiFiSense::sendMqtt(String msg)
{
  String payload = WiFi.macAddress() + MSG_SEPARATOR + msg;
  mqtt.publish(MQTT_TOPIC_SENSE_TO_BROKER, MQTT_QOS, false, payload.c_str());
}

void connectToWifi() {
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Connecting to Wi-Fi...");
  #endif
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Connected to Wi-Fi.");
  #endif
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Disconnected from Wi-Fi.");
  #endif
  mqttReconnectTimer.detach();
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Connecting to MQTT...");
  #endif
  mqtt.connect();
}

void onMqttConnect(bool sessionPresent) {
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Connected to MQTT.");
  SERIAL_DEBUG.print("Session present: ");
  SERIAL_DEBUG.println(sessionPresent);
  #endif
  wifiSense.updateMqtt();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Disconnected from MQTT.");
  #endif

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId)
{
  #ifdef SERIAL_DEBUG
  SERIAL_DEBUG.println("Going to Sleep");
  #endif
  ESP.deepSleep(DEEP_SLEEP_TIME);
}