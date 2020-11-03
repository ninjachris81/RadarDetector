// Wemos D1 R1

//#define NO_LOG_SERIAL

#include "TaskManager.h"
#include "MqttController.h"
#include "RadarController.h"
#include "credentials.h"

#include <LogHelper.h>

TaskManager taskManager;
MqttController mqttController;
RadarController radarController;

void onConnectionEstablished() {}   // legacy

void setup() {
  LOG_INIT();

  taskManager.registerTask(&mqttController);
  
  taskManager.registerTask(&radarController);

  taskManager.init();
  
  LOG_PRINTLN(F("INIT COMPLETE"));
}

void loop() {
  taskManager.update();
  delay(10);
}
