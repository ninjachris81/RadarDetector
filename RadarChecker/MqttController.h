#ifndef MQTTCONTROLLER_H
#define MQTTCONTROLLER_H

#include <AbstractIntervalTask.h>
#include "credentials.h"
#include "config.h"
#include "EspMQTTClient.h"
#include "constants.h"


#define MQTT_INTERVAL_MS 100
#define MAX_MQTT_EVENT_CB_HANDLERS 8

#define BUILD_PATH(subpath) MQTT_BASE_PATH + String(MQTT_PATH_SEP) + subpath

class MqttController : public AbstractIntervalTask {
public:
  MqttController();
  ~MqttController();

  class MqttEventCallbackHandler {
    public:
      virtual void onConnected() = 0;
  };
  
  void init();

  void update();

  String getIP();

  bool isConnected() {
    if (mqttClient) {
      return mqttClient->isConnected();
    } else {
      return false;
    }
  }

  static void onConnectionEstablishedStatic();
  void onConnectionEstablished();

  void registerHandler(MqttEventCallbackHandler* handler);

  void sendMessage(String path, String payload);
  void sendMessage(String path, bool payload);
  void sendMessage(String path, int payload);
  void sendMessage(String path, double payload);


private:
  static MqttController* instance;
  EspMQTTClient *mqttClient = 0;

  MqttEventCallbackHandler* callbackHandlers[MAX_MQTT_EVENT_CB_HANDLERS];
  uint8_t callbackHandlerCount = 0;
};


#endif
