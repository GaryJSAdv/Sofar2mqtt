

#ifndef configh
#define configh

#include "my_details.h"

#ifndef wifi_ssid
#define wifi_ssid  "SSID"  // You will connect your phone to this Access Point
#define wifi_pw    "PASSWORD" // and this is the password
#endif

//The divice name is used as the MQTT base topic. If you need more than one Sofar2mqtt on your network, give them unique names.
#define deviceName "Sofar2mqtt"
#define VERSION    "1.00"


#define LED_BUILTIN 2

#define DISP_SSD1306 1
#define DISP_SH1106 2
//#define DISPLAY_TYPE DISP_SSD1306
#define DISPLAY_TYPE  DISP_SH1106

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels


#define RS485_SERIAL_CHANNEL   1
#define SERIAL_COMMUNICATION_CONTROL_PIN 4 // Transmission set pin
#define RE_CONTROL  23 //rx485 re pin. Inverted signal. 

#define DEBUG_COM 0                 // debug output to COM0
/*************************  COM Port 0 *******************************/
#define UART_BAUD0 19200            // Baudrate UART0
#define SERIAL_PARAM0 SERIAL_8N1    // Data/Parity/Stop UART0
#define SERIAL0_RXPIN 21            // receive Pin UART0
#define SERIAL0_TXPIN 1             // transmit Pin UART0
#define SERIAL0_TCP_PORT 8880         // Wifi Port UART0
/*************************  COM Port 1 *******************************/
#define UART_BAUD1 9600            // Baudrate UART1
#define SERIAL_PARAM1 SERIAL_8N1    // Data/Parity/Stop UART1
#define SERIAL1_RXPIN 16            // receive Pin UART1
#define SERIAL1_TXPIN 17            // transmit Pin UART1
/*************************  COM Port 2 *******************************/
#define UART_BAUD2 19200            // Baudrate UART2
#define SERIAL_PARAM2 SERIAL_8N1    // Data/Parity/Stop UART2
#define SERIAL2_RXPIN 15            // receive Pin UART2
#define SERIAL2_TXPIN 4             // transmit Pin UART2


//////////////////////////////////////////////////////////////////////////
#endif