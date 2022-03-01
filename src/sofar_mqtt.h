#ifndef sofarmqtth
#define sofarmqtth

#include <Arduino.h>
#include "config.h"
#include "PubSubClient.h"


// MQTT parameters
#define mqttClientID  deviceName
#define mqttServer  "mqtt"
#define mqttPort  1883
#define  mqttUsername  "auser"
#define  mqttPassword  "apassword"

extern void mqttReconnect(void);

extern void sendMqtt(char* topic, String msg_str);

#endif