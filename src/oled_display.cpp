
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "globals.h"
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
		display.drawFastHLine(0,9,128, WHITE);
	}
	else 
	{
		display.println(oledLine1);
		display.drawFastHLine(0,9,128, WHITE);

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

    //status area
    

const uint8_t battery_bmp[] = {0xFF,0xFF,0xFC,
	                         0xFF,0xFF,0xFC,
							 0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFC,
							 0xFF,0xFF,0xFC};

const uint8_t grid_bmp[] = {
  0x10, 
  0x38, 
  0xfe, 
  0x38, 
  0x7c, 
  0xfe, 
  0x38, 
  0x38, 
  0x6c, 
  0x44, 
  0x82, 
};

const uint8_t solar_bmp[] = {0xF9,0x9F,
	                         0xF9,0x9F,
							 0xF9,0x9F,
							 0x00,0x00,
							 0xF9,0x9F,
							 0xF9,0x9F,
							 0x00,0x00,
							 0xF9,0x9F,
							 0xF9,0x9F,
							 0xF9,0x9F};



	display.drawBitmap (70,17, battery_bmp, 24, 9, WHITE);
	display.setCursor(100,12);
	display.println(String(sys_status.battery_charge)+"%");
	display.setCursor(100,24);
	display.println(String(sys_status.battery_power)+"W");


	display.setCursor(100,36);
	display.drawBitmap(75,34, solar_bmp, 16, 10, WHITE);
	display.println(String(sys_status.solar_power)+"W");

	display.setCursor(100,48);
	display.drawBitmap (80,46, grid_bmp, 7, 11, WHITE);
	display.println(String(sys_status.grid_power)+"W");

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