#ifndef RADARCONTROLLER_H
#define RADARCONTROLLER_H

#include <AbstractIntervalTask.h>

#include "TaskIDs.h"
#include "Pins.h"
#include "MqttController.h"

#define PROP_STATE 0

#define MQTT_PATH_RADAR_CHECKER "radar"

#define MQTT_PATH_RADAR_CHECKER_STATE 0

#define RADAR_INTERVAL_MS 500

#define MQTT_UPDATE_INTERVAL_MS 10000

class RadarController : public AbstractIntervalTask, public MqttController::MqttEventCallbackHandler {
public:
  RadarController();
  ~RadarController();

  void init();

  void update();
  
  void onConnected();

  void sendStatus();

private:
  uint64_t lastMqttUpdate = 0;

  int radarState = 0;

};


#endif
