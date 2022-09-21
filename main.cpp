#include "esp_system.h" //This inclusion configures the peripherals in the ESP system.
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>

#define PIN 1
#define NUMPIXELS 252

TaskHandle_t PixelHandle;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int twodots = 0;
String aux = "12 34";

void setupWifi();
void setupOta();
void pixelDraw();
void loop0(void *parameter);

void setup()
{
  delay(2000);
  Serial.begin(9600);
  SPIFFS.begin(true);

  setupWifi();
  // pixels.begin();

  // xTaskCreatePinnedToCore(loop0, "Pixels", 10000, NULL, 0, &PixelHandle, 0);
}

void setupOta()
{
  ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
  //ArduinoOTA.setPassword(WiFiSettings.password.c_str());
  //ArduinoOTA.setPort(3232);
  ArduinoOTA.begin();
}

void setupWifi()
{
  // Force WPA secured WiFi for the software access point.
  // Because OTA is remote code execution (RCE) by definition, the password
  // should be kept secret. By default, WiFiSettings will become an insecure
  // WiFi access point and happily tell anyone the password. The password
  // will instead be provided on the Serial connection, which is a bit safer.
  //WiFiSettings.secure = true;
  WiFiSettings.hostname = "GiantClock";

  // Set callbacks to start OTA when the portal is active
  WiFiSettings.onPortal = []()
  {
    setupOta();
  };

  WiFiSettings.onPortalWaitLoop = []()
  {
    ArduinoOTA.handle();
  };

  // Use stored credentials to connect to your WiFi access point.
  // If no credentials are stored or if the access point is out of reach,
  // an access point will be started with a captive portal to configure WiFi.
  WiFiSettings.connect(true, -1);

  Serial.print("Password: ");
  Serial.println(WiFiSettings.password);
}

void loop()
{
  ArduinoOTA.handle();
}

void loop0(void *parameter) {
  for(;;) {
    pixelDraw();
  }
}

void pixelDraw()
{
  //Serial.print("hello, starting now...\n");

  if (twodots > 0)
  {
    twodots = 0;
  }
  else
  {
    twodots = 255;
  }

  for (int i = 0; i < NUMPIXELS; i++)
  {
    pixels.setPixelColor(i, pixels.Color(twodots, 0, 0));
    //delay(250);
  }
}

// // START OF CODE************************************************************

// /*

// WifiSettings basic example

// Source and further documentation available at
// https://github.com/Juerd/ESP-WiFiSettings

// Note: this example is written for ESP32.
// For ESP8266, use LittleFS.begin() instead of SPIFFS.begin(true).
// */
// // NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// // released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library
// #include <NTPClient.h>
// #include <WiFiUdp.h>
// #include <Adafruit_NeoPixel.h>
// #include <LittleFS.h>
// #include <WiFiUdp.h>
// #include <WiFiSettings.h>

// #define PIN 4
// #define NUMPIXELS 450

// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "192.168.7.3", 3600, 60000);

// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// int twodots = 0;

// int scanner[68];

// int scannercounter = 0;

// void setup()
// {
//   Serial.begin(115200);
//   // SPIFFS.begin(true);
//   // WiFiSettings.hostname = "GiantClock";
//   // WiFiSettings.connect(false, -1);
//   delay(500);
//   pixels.begin();
//   // timeClient.begin();
// }

// void emptyMatrix()
// {
//   for (int j = 67; j < 400; j++)
//   {
//     pixels.setPixelColor(j, pixels.Color(0, 0, 0));
//   }
// }

// void segment(int start, int count = 9, int red = 0, int green = 0, int blue = 0)
// {
//   for (int i = start; i < start + count; i++)
//   {
//     pixels.setPixelColor(i, pixels.Color(red, green, blue));
//   }
// }

// void digit(int number, int shift, int red, int green, int blue)
// {

//   // UPPER SEGMENT
//   if (number == 0 || number == 2 || number == 3 || number == 5 || number == 6 || number == 7 || number == 8 || number == 9)
//   {
//     segment(36 + shift, 9, red, green, blue);

//     // pixels.setPixelColor(36 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(37 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(38 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(39 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(40 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(41 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(42 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(43 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(44 + shift, pixels.Color(red, green, blue));
//   }

//   // UPPERLEFTSEGMENT
//   if (number == 0 || number == 4 || number == 5 || number == 6 || number == 8 || number == 9)
//   {
//     segment(27 + shift, 9, red, green, blue);

//     // pixels.setPixelColor(27 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(28 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(29 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(30 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(31 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(32 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(33 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(34 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(35 + shift, pixels.Color(red, green, blue));
//   }

//   // UPPERRIGHTSEGMENT
//   if (number == 0 || number == 1 || number == 2 || number == 3 || number == 4 || number == 7 || number == 8 || number == 9)
//   {
//     segment(45 + shift, 9, red, green, blue);

//     // pixels.setPixelColor(45 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(46 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(47 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(48 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(49 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(50 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(51 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(52 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(53 + shift, pixels.Color(red, green, blue));
//   }

//   // MIDDLESEGMENT
//   if (number == 2 || number == 3 || number == 4 || number == 5 || number == 6 || number == 8 || number == 9)
//   {
//     segment(54 + shift, 9, red, green, blue);

//     // pixels.setPixelColor(54 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(55 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(56 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(57 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(58 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(59 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(60 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(61 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(62 + shift, pixels.Color(red, green, blue));
//   }

//   // LOWERLEFTSEGMENT
//   if (number == 0 || number == 2 || number == 6 || number == 8)
//   {
//     segment(18 + shift, 9, red, green, blue);

//     // pixels.setPixelColor(18 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(19 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(20 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(21 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(22 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(23 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(24 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(25 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(26 + shift, pixels.Color(red, green, blue));
//   }

//   // LOWERRIGHTSEGMENT
//   if (number == 0 || number == 1 || number == 3 || number == 4 || number == 5 || number == 6 || number == 7 || number == 8 || number == 9)
//   {
//     segment(shift, 9, red, green, blue);

//     // pixels.setPixelColor(0 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(1 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(2 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(3 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(4 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(5 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(6 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(7 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(8 + shift, pixels.Color(red, green, blue));
//   }

//   // LOWERSEGMENT
//   if (number == 0 || number == 2 || number == 3 || number == 5 || number == 6 || number == 8 || number == 9)
//   {
//     segment(9 + shift, 9, red, green, blue);

//     // pixels.setPixelColor(9 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(10 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(11 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(12 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(13 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(14 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(15 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(16 + shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(17 + shift, pixels.Color(red, green, blue));
//   }

//   // This sends the updated pixel color to the hardware.
// }

// void twodotos(int shift, int red, int green, int blue)
// {
//   if (twodots == 0)
//   {
//     segment(shift, 6, red, green, blue);
//     twodots = 1;

//     // pixels.setPixelColor(shift, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(shift + 1, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(shift + 2, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(shift + 3, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(shift + 4, pixels.Color(red, green, blue));
//     // pixels.setPixelColor(shift + 5, pixels.Color(red, green, blue));
//   }
//   else
//   {
//     segment(shift, 6, 0, 0, 0);
//     twodots = 0;

//     // pixels.setPixelColor(shift, pixels.Color(0, 0, 0));
//     // pixels.setPixelColor(shift + 1, pixels.Color(0, 0, 0));
//     // pixels.setPixelColor(shift + 2, pixels.Color(0, 0, 0));
//     // pixels.setPixelColor(shift + 3, pixels.Color(0, 0, 0));
//     // pixels.setPixelColor(shift + 4, pixels.Color(0, 0, 0));
//     // pixels.setPixelColor(shift + 5, pixels.Color(0, 0, 0));
//   }
// }

// void scannerdrawplus(int red, int green, int blue)
// {
//   for (int w = 0; w < 28; w++)
//   {
//     for (int i = 0; i < 68; i++)
//     {
//       pixels.setPixelColor(i, pixels.Color(0, 0, 0));
//     }
//     pixels.setPixelColor(w, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 1, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 2, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 3, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 4, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 5, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 1, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 2, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 3, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 4, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 5, pixels.Color(red, green, blue));
//     pixels.show();
//     delay(10);
//   }
//   for (int w = 27; w >= 0; w--)
//   {
//     for (int i = 0; i < 68; i++)
//     {
//       pixels.setPixelColor(i, pixels.Color(0, 0, 0));
//     }
//     pixels.setPixelColor(w, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 1, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 2, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 3, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 4, pixels.Color(red, green, blue));
//     pixels.setPixelColor(w + 5, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 1, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 2, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 3, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 4, pixels.Color(red, green, blue));
//     pixels.setPixelColor(67 - w + 5, pixels.Color(red, green, blue));

//     pixels.show();
//     delay(10);
//   }
// }

// void colorea(int red, int green, int blue)
// {
//   Serial.println(timeClient.getFormattedTime());
//   //String aux = timeClient.getFormattedTime();
//   String aux = "12 34";
//   digit(int(aux.charAt(0)) - 48, 68, red, green, blue);
//   digit(int(aux.charAt(1)) - 48, 68 + (9 * 7), red, green, blue);
//   twodotos(68 + (9 * 7 * 2), red, green, blue);
//   digit(int(aux.charAt(3)) - 48, 68 + (9 * 7 * 2) + 6, red, green, blue);
//   digit(int(aux.charAt(4)) - 48, 68 + (9 * 7 * 3) + 6, red, green, blue);
//   pixels.show();
// }
// void individual()
// {
//   Serial.println(timeClient.getFormattedTime());
//   String aux = timeClient.getFormattedTime();
//   digit(int(aux.charAt(0)) - 48, 68, 255, 0, 0);             // HOUR TENTHS
//   digit(int(aux.charAt(1)) - 48, 68 + (9 * 7), 255, 0, 255); // HOUR UNITS
//   twodotos(68 + (9 * 7 * 2), 0, 0, 255);
//   digit(int(aux.charAt(3)) - 48, 68 + (9 * 7 * 2) + 6, 0, 255, 255); // MINUTE TENTHS
//   digit(int(aux.charAt(4)) - 48, 68 + (9 * 7 * 3) + 6, 0, 255, 0);   // MINUTE UNITS
//   pixels.show();
// }

// void loop()
// {

//   timeClient.update();
//   emptyMatrix();
//   colorea(255, 0, 0);
//   // individual();
//   //scannerdrawplus(255, 0, 0);

//   //delay(1000);
// }

// // END OF CODE**********************************************