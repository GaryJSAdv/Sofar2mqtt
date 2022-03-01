#include <Arduino.h>
#include <esp_wifi.h>
#include <Wifi.h>

#include "config.h"

WiFiClient wifi;

// Connect to WiFi
void setup_wifi(void) 
{
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(wifi_ssid);
	//updateOLED("NULL", "NULL", "WiFi..", "NULL");
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_ssid, wifi_pw);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
		//updateOLED("NULL", "NULL", "WiFi...", "NULL");
	}
	WiFi.hostname(deviceName);
	Serial.println("");
	Serial.print("WiFi connected - ESP IP address: ");
	Serial.println(WiFi.localIP());
	//updateOLED("NULL", "NULL", "WiFi....", "NULL");
}