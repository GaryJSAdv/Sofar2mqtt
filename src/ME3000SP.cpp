/*****
Sofar2mqtt is a remote control interface for Sofar solar and battery inverters.
It allows remote control of the inverter and reports the invertor status, power usage, battery state etc for integration with smart home systems such as Home Assistant and Node-Red vi MQTT.  
For read only mode, it will send status messages without the inverter needing to be in passive mode.  
It's designed to run on an ESP8266 microcontroller with a TTL to RS485 module such as MAX485 or MAX3485.  
Designed to work with TTL modules with or without the DR and RE flow control pins. If your TTL module does not have these pins then just ignore the wire from D5. 

Subscribe your MQTT client to:

sofar2mqtt/state

Which provides:

running_state  
grid_voltage  
grid_current  
grid_freq  
battery_power  
battery_voltage  
battery_current  
batterySOC  
battery_temp  
battery_cycles  
grid_power  
consumption  
solarPV  
today_generation  
today_exported  
today_purchase  
today_consumption  
inverter_temp  
inverterHS_temp  
solarPVAmps  

With the inverter in Passive Mode, send MQTT messages to:

sofar2mqtt/set/standby   - send value "true"  
sofar2mqtt/set/auto   - send value "true" or "battery_save"  
sofar2mqtt/set/charge   - send values in the range 0-3000 (watts)  
sofar2mqtt/set/discharge   - send values in the range 0-3000 (watts) 

battery_save is a hybrid auto mode that will charge from excess solar but not discharge.

(c)Colin McGerty 2021 colin@mcgerty.co.uk
Thanks to Rich Platts for hybrid model code and testing.
calcCRC by angelo.compagnucci@gmail.com and jpmzometa@gmail.com
*****/


#include <Arduino.h>
#include <esp_wifi.h>
#include <Wifi.h>


#include "config.h"
#include "sofar_modbus.h"
#include "sofar_mqtt.h"
#include "sofar_wifi.h"
#include "oled_display.h"



// Sofar run states

#define waiting 0
#define check 1
#define charging 2
#define checkDischarge 3
#define discharging 4
#define epsState 5
#define faultState 6
#define permanentFaultState 7
unsigned int INVERTER_RUNNINGSTATE;
// Battery Save mode is a hybrid mode where the battery will charge from excess solar but not discharge.
bool BATTERYSAVE = false;


// These timers are used in the main loop.
unsigned long time_1 = 0;
unsigned long led_heartbeat_time = 0;
#define HEARTBEAT_INTERVAL 9000
#define LED_HEARTBEAT_INTERVAL 500
unsigned long time_2 = 0;
#define RUNSTATE_INTERVAL 5000


unsigned long time_4 = 0;
#define BATTERYSAVE_INTERVAL 3000


void batterySave()
{
	if(millis() >= time_4 + BATTERYSAVE_INTERVAL)
	{
		time_4 +=BATTERYSAVE_INTERVAL;
		if (BATTERYSAVE)
		{
			//Get grid power
			TModbusResponce gp = sendModbus(getGridPower, sizeof(getGridPower));
			unsigned int p = 0;
			if (gp.errorLevel == 0)
			{
				p = ((gp.data[0] << 8) | gp.data[1]);
			}
			else
			{
				Serial.println("modbus error");
			}
			Serial.print("Grid power: ");
			Serial.println(p);
			Serial.print("Battery save mode: ");
			// Switch to auto when any power flows to the grid.
			// We leave a little wriggle room because once you start charging the battery, 
			// gridPower should be floating just above or below zero.
			if (p<65535/2 || p>65525)
			{
				//exporting to the grid
				TModbusResponce responce = sendModbus(setAuto, sizeof(setAuto));
				if (responce.errorLevel == 0)
				{
					Serial.println("auto");
				}
			}
			else
			{
				//importing from the grid
				TModbusResponce responce = sendModbus(setStandby, sizeof(setStandby));
				if (responce.errorLevel == 0)
				{
					Serial.println("standby");
				}
			}
		}
	}
}

void heartbeat(void)
{
uint32_t time_now = millis();
static bool     alternate = false;
static bool     error_last_pass = false;
	
	//Send a heartbeat
	if(time_now >= time_1 + HEARTBEAT_INTERVAL)
	{
		time_1 += HEARTBEAT_INTERVAL;
		
		Serial.println("Send heartbeat");
		TModbusResponce responce = sendModbus(sendHeartbeat, sizeof(sendHeartbeat));
		
		// This just makes the dot on the first line of the OLED screen flash on and off with 
		// the heartbeat and clears any previous RS485 error massage that might still be there.
		if (responce.errorLevel == 0)
		{
			String flashDot;
			alternate = !alternate; 

			if (alternate == true) 
				flashDot = "Online.";
			else
				flashDot = "Online";
				
            if (error_last_pass)
			{
			  updateOLED("NULL", flashDot, "NULL", "NULL");
			}
			else
			{
			  //clear the error message
			  updateOLED("", flashDot, "", "");
			}
		}
		else
		{
			error_last_pass = true;
			Serial.println(responce.errorMessage);
			updateOLED("NULL", "NULL", "RS485", "ERROR");
		}
	}
	if (time_now > led_heartbeat_time)
	{
		led_heartbeat_time = time_now + LED_HEARTBEAT_INTERVAL;
		//Toggle the LED
		digitalWrite (LED_BUILTIN, !digitalRead(LED_BUILTIN));
	}
}

unsigned int batteryWatts()
{	
	if (INVERTER_RUNNINGSTATE == charging || INVERTER_RUNNINGSTATE == discharging)
	{
		TModbusResponce responce = sendModbus(getBatteryPower, sizeof(getBatteryPower));
		if (responce.errorLevel == 0)
		{
			unsigned int w = ((responce.data[0] << 8) | responce.data[1]);
			switch (INVERTER_RUNNINGSTATE) {
				case charging:
					w = w*10;
					break;
				case discharging:
					w = (65535 - w)*10;
					break;
			}
			return w;
		}
		else
		{
			Serial.println(responce.errorMessage);
			updateOLED("NULL", "NULL", "CRC-FAULT", "NULL");
		}
	}
	return 0;
}

void updateRunstate()
{
	//Check the runstate
	if(millis() >= time_2 + RUNSTATE_INTERVAL)
	{
		time_2 +=RUNSTATE_INTERVAL;
		Serial.print("Get runstate: ");
		TModbusResponce responce = sendModbus(getRunningState, sizeof(getRunningState));
		if (responce.errorLevel == 0)
		{
			INVERTER_RUNNINGSTATE = ((responce.data[0] << 8) | responce.data[1]);
			Serial.println(INVERTER_RUNNINGSTATE);
			switch (INVERTER_RUNNINGSTATE) {
				case waiting:
				{
					if (BATTERYSAVE)
					{
						updateOLED("NULL", "NULL", "Batt Save", "Waiting");
					}
					else
					{
						updateOLED("NULL", "NULL", "Standby", "");
					}
					break;
				}
				case check:
					updateOLED("NULL", "NULL", "Checking", "NULL");
					break;
				case charging:
					updateOLED("NULL", "NULL", "Charging", String(batteryWatts())+"W");
					break;
				case checkDischarge:
					updateOLED("NULL", "NULL", "Check Dis", "NULL");
					break;
				case discharging:
					updateOLED("NULL", "NULL", "Discharge", String(batteryWatts())+"W");
					break;
				case epsState:
					updateOLED("NULL", "NULL", "EPS State", "NULL");
					break;
				case faultState:
					updateOLED("NULL", "NULL", "FAULT", "NULL");
					break;
				case permanentFaultState:
					updateOLED("NULL", "NULL", "PERMFAULT", "NULL");
					break;
				default:
					updateOLED("NULL", "NULL", "Runstate?", "NULL");
					break;
			}
		}
		else
		{
			Serial.println(responce.errorMessage);
			updateOLED("NULL", "NULL", "CRC-FAULT", "NULL");
		}
	}
}

void setup()
{
	Serial.begin(19200);
	Serial.println ("Hello!");
	pinMode(LED_BUILTIN, OUTPUT);
	
	InitSofarModbus ();
	
	delay(500);

    InitOledDisplay();

	setup_wifi();
	
	InitMqtt ();
	
	//Wake up the inverter and put it in auto mode to begin with.
	heartbeat();
	Serial.println("Set start up mode: Auto");
	sendModbus(setAuto, sizeof(setAuto));	
}

void loop()
{
	//make sure mqtt is still connected
	CheckMqttConnected ();
	
	//check mqtt for incomming messages
	CheckForNewMqttMessages();
	
	//Send a heartbaet to keep the inverter awake
	heartbeat();
	//Check and display the runstate
	/*updateRunstate();
	//Transmit all data to MQTT
	sendData();
	//Set battery save state
	batterySave();*/

	delay(100);
}

