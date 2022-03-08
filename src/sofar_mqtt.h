#ifndef sofarmqtth
#define sofarmqtth

#include <Arduino.h>
#include "config.h"
#include "PubSubClient.h"


// MQTT parameters
#define mqttClientID  deviceName
#define mqttServer  "test.mosquitto.org"
#define mqttPort  1883
#define  mqttUsername  NULL//"auser"
#define  mqttPassword  NULL//"apassword"

extern void mqttReconnect(void);
extern void CheckMqttConnected (void);
extern void CheckForNewMqttMessages(void);
void InitMqtt (void);
extern void sendMqtt(char* topic, String msg_str);

#endif