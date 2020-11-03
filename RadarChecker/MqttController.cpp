#include "MqttController.h"
#include <LogHelper.h>

MqttController* MqttController::instance = NULL;

MqttController::MqttController() : AbstractIntervalTask(MQTT_INTERVAL_MS) {
  
}

MqttController::~MqttController() {
  delete mqttClient;
}

void MqttController::init() {
  instance = this;
  
  mqttClient = new EspMQTTClient(
    WIFI_SSID,                 // Wifi ssid
    WIFI_PASS,                 // Wifi password
    &MqttController::onConnectionEstablishedStatic,// MQTT connection established callback
    MQTT_SERVER,                    // MQTT broker ip
    MQTT_PORT,
    "",
    "",
    DEVICE_NAME,
    false
  );

  mqttClient->enableHTTPWebUpdater();
}

void MqttController::update() {
  mqttClient->loop();
}

void MqttController::onConnectionEstablishedStatic() {
  instance->onConnectionEstablished();
}

void MqttController::onConnectionEstablished() {
  for (uint8_t i=0;i<callbackHandlerCount;i++) callbackHandlers[i]->onConnected();
}

String MqttController::getIP() {
  return WiFi.localIP().toString();
}

void MqttController::registerHandler(MqttEventCallbackHandler* handler) {
  callbackHandlers[callbackHandlerCount] = handler;
  callbackHandlerCount++;
}

void MqttController::sendMessage(String path, String payload) {
  if (mqttClient->isConnected()) {
    payload = String(MQTT_ID_STRING) + payload;
    mqttClient->publish(path, payload);
  }
}

void MqttController::sendMessage(String path, bool payload) {
  if (mqttClient->isConnected()) {
    mqttClient->publish(path, String(MQTT_ID_BOOL) + (payload ? String("1") : String("0")));
  }
}

void MqttController::sendMessage(String path, int payload) {
  if (mqttClient->isConnected()) {
    mqttClient->publish(path, String(MQTT_ID_INTEGER) + payload);
  }
}

void MqttController::sendMessage(String path, double payload) {
  if (mqttClient->isConnected()) {
    mqttClient->publish(path, String(MQTT_ID_DOUBLE) + String(payload));
  }
}
