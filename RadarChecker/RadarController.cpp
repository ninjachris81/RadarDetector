#include "RadarController.h"
#include <LogHelper.h>

RadarController::RadarController() : AbstractIntervalTask(RADAR_INTERVAL_MS) {
  
}

RadarController::~RadarController() {
}

void RadarController::init() {
  taskManager->getTask<MqttController*>(MQTT_CONTROLLER)->registerHandler(this);

  pinMode(PIN_RADAR, INPUT);
  //ledcSetup(0, 500, 8);
  //ledcAttachPin(PIN_BUZZER, 0);
}

void RadarController::update() {
  if (!taskManager->getTask<MqttController*>(MQTT_CONTROLLER)->isConnected()) return;

  if (digitalRead(PIN_RADAR)) {
    radarState++;
    //ledcWriteTone(0, 500);
  } else {
    //ledcWriteTone(0, 0);
  }

  if (lastMqttUpdate==0 || millis() - lastMqttUpdate > MQTT_UPDATE_INTERVAL_MS) {
    lastMqttUpdate = millis();
    sendStatus();
  }
}

void RadarController::onConnected() {
  LOG_PRINTLN(F("MQTT connected"));
  sendStatus();
}

void RadarController::sendStatus() {
  if (!taskManager->getTask<MqttController*>(MQTT_CONTROLLER)->isConnected()) return;

  taskManager->getTask<MqttController*>(MQTT_CONTROLLER)->sendMessage(BUILD_PATH(MQTT_PATH_RADAR_CHECKER + String(MQTT_PATH_SEP) + MQTT_VAL + String(MQTT_PATH_SEP) + String(MQTT_PATH_RADAR_CHECKER_STATE)), radarState);

  radarState = 0;
}
