
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "config.h"

	

#define OLED_RESET -1  	

#if (DISPLAY_TYPE==DISP_SSD1306)
  #include <Adafruit_SSD1306.h>
  #define DISPLAY_POWER SSD1306_SWITCHCAPVCC
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#elif (DISPLAY_TYPE==DISP_SH1106)
#include "Adafruit_SH1106.h"
  #define DISPLAY_POWER SH1106_SWITCHCAPVCC
  Adafruit_SH1106 display(OLED_RESET);
#endif


// Update the OLED. Use "NULL" for no change or "" for an empty line.
String oledLine1;
String oledLine2;
String oledLine3;
String oledLine4;
  

void updateOLED(String line1, String line2, String line3, String line4) 
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);

	display.setCursor(0,0);
	if (line1 != "NULL") 
	{
		display.println(line1);
		oledLine1 = line1;
	}
	else 
	{
		display.println(oledLine1);
	}
	display.setCursor(0,12);
	if (line2 != "NULL") 
	{
		display.println(line2);
		oledLine2 = line2;
	}
	else 
	{
		display.println(oledLine2);
	}
	display.setCursor(0,24);
	if (line3 != "NULL") 
	{
		display.println(line3);
		oledLine3 = line3;
	}
	else 
	{
		display.println(oledLine3);
	}
	display.setCursor(0,36);
	if (line4 != "NULL") 
	{
		display.println(line4);
		oledLine4 = line4;
	}
	else 
	{
		display.println(oledLine4);
	}
	display.display();
}


void InitOledDisplay (void)
{
	//Turn on the OLED
	display.begin(DISPLAY_POWER, 0x3C);  // initialize OLED with the I2C addr 0x3C (for the 64x48)
	display.clearDisplay();
	display.display();
	updateOLED(deviceName, "connecting", "", VERSION);
}  