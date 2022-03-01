#include "sofar_mqtt.h"
#include "sofar_modbus.h"
#include "oled_display.h"
#include "sofar_wifi.h"
#include "globals.h"


PubSubClient mqtt(wifi);


void InitMqtt (void)
{

}



// This function is executed when an MQTT message arrives on a topic that we are subscribed to.
void mqttCallback(String topic, byte* message, unsigned int length) 
{
	Serial.print("Message arrived on topic: ");
	Serial.print(topic);
	Serial.print(". Message: ");
	String messageTemp;

	for (int i = 0; i < length; i++) 
	{
		Serial.print((char)message[i]);
		messageTemp += (char)message[i];
	}

	Serial.println();
	int messageValue = messageTemp.toInt();

	//Set topic names to include the deviceName.
	String standbyMode (deviceName);
	standbyMode += "/set/standby";
	String autoMode (deviceName);
	autoMode += "/set/auto";
	String chargeMode (deviceName);
	chargeMode += "/set/charge";
	String dischargeMode (deviceName);
	dischargeMode += "/set/discharge";
	
	// This is where we look at incoming messages and take action based on their content.
	if (topic==standbyMode)
	{
		BATTERYSAVE = false;
		if(messageTemp == "true")
		{
			TModbusResponce responce = sendModbus(setStandby, sizeof(setStandby));
			if (responce.errorLevel == 0)
			{
				Serial.println(responce.errorMessage);
			}
		}
	}
	else if (topic==autoMode)
	{
		if(messageTemp == "true")
		{
			BATTERYSAVE = false;
			TModbusResponce responce = sendModbus(setAuto, sizeof(setAuto));
			if (responce.errorLevel == 0)
			{
				Serial.println(responce.errorMessage);
			}
		}
		else if(messageTemp == "battery_save")
		{
			BATTERYSAVE = true;
		}
	}
	else if (topic==chargeMode)
	{
		if(messageTemp != "false")
		{
			BATTERYSAVE = false;
			if (messageValue > 0 && messageValue < 3001)
			{
				setCharge[4] = highByte(messageValue);
				setCharge[5] = lowByte(messageValue);
				TModbusResponce responce = sendModbus(setCharge, sizeof(setCharge));
				if (responce.errorLevel == 0)
				{
					Serial.println(responce.errorMessage);
				}
			}
		}
	}
	else if (topic==dischargeMode)
	{
		if(messageTemp != "false")
		{
			BATTERYSAVE = false;
			if (messageValue > 0 && messageValue < 3001)
			{
				setDischarge[4] = highByte(messageValue);
				setDischarge[5] = lowByte(messageValue);
				TModbusResponce responce = sendModbus(setDischarge, sizeof(setDischarge));
				if (responce.errorLevel == 0)
				{
					Serial.println(responce.errorMessage);
				}
			}
		}
	}
}


// This function reconnects the ESP8266 to the MQTT broker
void mqttReconnect(void) 
{
	// Loop until we're reconnected
	while (!mqtt.connected()) 
	{
		Serial.print("Attempting MQTT connection...");
		updateOLED("NULL", "connecting", "NULL", "MQTT.");
		delay(500);
		updateOLED("NULL", "NULL", "NULL", "MQTT..");
		// Attempt to connect
		if (mqtt.connect(mqttClientID, mqttUsername, mqttPassword)) 
		{
			Serial.println("connected");
			delay(1000);
			updateOLED("NULL", "NULL", "NULL", "MQTT....");
			delay(1000);
			
			//Set topic names to include the deviceName.
			String standbyMode (deviceName);
			standbyMode += "/set/standby";
			String autoMode (deviceName);
			autoMode += "/set/auto";
			String chargeMode (deviceName);
			chargeMode += "/set/charge";
			String dischargeMode (deviceName);
			dischargeMode += "/set/discharge";
	
			// Subscribe or resubscribe to topics.
			mqtt.subscribe(const_cast<char*>(standbyMode.c_str()));
			mqtt.subscribe(const_cast<char*>(autoMode.c_str()));
			mqtt.subscribe(const_cast<char*>(chargeMode.c_str()));
			mqtt.subscribe(const_cast<char*>(dischargeMode.c_str()));
			updateOLED("NULL", "NULL", "NULL", "");
		} 
		else 
		{
			Serial.print("failed, rc=");
			Serial.print(mqtt.state());
			Serial.println(" try again in 5 seconds");
			updateOLED("NULL", "NULL", "NULL", "MQTT...");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}


void sendMqtt(char* topic, String msg_str)
{
	char msg[1000];
	mqtt.setBufferSize(512);
	msg_str.toCharArray(msg, msg_str.length() + 1); //packaging up the data to publish to mqtt
	if (!(mqtt.publish(topic, msg)))
	{
		Serial.println("MQTT publish failed");
	}	
	
}