#include "esp_system.h"

/**
 * @brief Create and include your own include/secret.h
 */
#include "secret.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

#include "FastLED.h"

#define NUM_LEDS 258
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define DATA_PIN 16

const int SEGMENT_PIXEL_COUNT = 9;
const int SEGMENT_PER_DIGIT = 7;
const int DOTS_COUNT = 6;
const int DIGIT_PIXEL_COUNT = SEGMENT_PIXEL_COUNT * SEGMENT_PER_DIGIT;
const StaticJsonDocument<2000> mqttMsq;
const SemaphoreHandle_t displaySem = xSemaphoreCreateBinary();
struct tm timeinfo;

enum class DigitOffset : int
{
    Digit_1 = 0,
    Digit_2 = Digit_1 + DIGIT_PIXEL_COUNT,
    Dots = Digit_2 + DIGIT_PIXEL_COUNT,
    Digit_3 = Dots + DOTS_COUNT,
    Digit_4 = Digit_3 + DIGIT_PIXEL_COUNT,
};

const DigitOffset DIGIT_OFFSETS[] = {DigitOffset::Digit_1, DigitOffset::Digit_2, DigitOffset::Digit_3, DigitOffset::Digit_4};

enum class Segment : int
{
    A = 0,
    B = A + SEGMENT_PIXEL_COUNT,
    C = B + SEGMENT_PIXEL_COUNT,
    D = C + SEGMENT_PIXEL_COUNT,
    E = D + SEGMENT_PIXEL_COUNT,
    F = E + SEGMENT_PIXEL_COUNT,
    G = F + SEGMENT_PIXEL_COUNT,
};

typedef void (*SegmentFunction)(DigitOffset, CRGB);
struct WordSegment
{
    String c;
    SegmentFunction segmentFunction;
};

CRGB leds[NUM_LEDS];

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

/**
 * @brief Create and include your own include/secret.h
 */
const char *ssid = SSID;
const char *password = WIFI_PASSWORD;
const char *hostname = HOSTNAME;

const IPAddress mqttBroker(192, 168, 7, 80);
const int mqttPort = 1883;
const char *mqttUser = MQTT_USER;
const char *mqttPass = MQTT_PASS;
const char topic[] = "";

TaskHandle_t PixelHandle;
TaskHandle_t UpdateHandle;
TaskHandle_t MqttHandle;

void updateLoop(void *param);

void pixelSetup();
void pixelLoop(void *param);

void mqttSetup();
void mqttLoop(void *param);
void pixelDraw();

void wifiSetup();

void arduinoOta();
void arduinoOtaStart();
void arduinoOtaEnd();
void arduinoOtaProgress(unsigned int progress, unsigned int total);
void arduinoOtaError(ota_error_t error);

void onMqttMessage(int messageSize);

void wrd(String wrd, CRGB rgb);
void drawTime(CRGB rgb);

bool updating = false;

void setup()
{
    delay(3000);

    configTzTime("PST8PDT,M3.2.0,M11.1.0", "192.168.7.3");

    Serial.begin(9600);
    Serial.print("hello, starting now... \n");
    SPIFFS.begin(true);

    if ((displaySem) != NULL)
        xSemaphoreGive(displaySem);

    pixelSetup();
    wifiSetup();

    // Setup the OTA update ASAP so if something goes wrong from here we can fix it.
    arduinoOta();
    mqttSetup();

    xTaskCreatePinnedToCore(updateLoop, "Update", 10000, NULL, 1, &UpdateHandle, 0);
    xTaskCreatePinnedToCore(mqttLoop, "Mqtt", 10000, NULL, 1, &MqttHandle, 0);
    xTaskCreatePinnedToCore(pixelLoop, "Pixels", 10000, NULL, 1, &PixelHandle, 1);
}

void loop() {}

template <typename withSem>
void withSemaphore(withSem f)
{
    while (xSemaphoreTake(displaySem, 10) != pdTRUE)
        ;

    try
    {
        f();
    }
    catch (const std::exception &e)
    {
    }

    xSemaphoreGive(displaySem);
}

void pixelSetup()
{
    Serial.println("pixelSetup");

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.clear();
    wrd("run", CRGB::White);
    FastLED.show();
    delay(3000);
}

bool wifiConnected()
{
    return WiFi.isConnected();
}

void wifiSetup()
{
    Serial.println("wifiSetup");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.println("Connection Failed! Rebooting...");
        ESP.restart();
    }
}

void mqttSetup()
{
    Serial.println("mqttSetup");

    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(mqttBroker);
    Serial.println("Setting MQTT user/pass");
    mqttClient.setUsernamePassword(mqttUser, mqttPass);
    mqttClient.setConnectionTimeout(500);
}

void updateLoop(void *param)
{
    auto lambda = []()
    { ArduinoOTA.handle(); };

    for (;;)
    {
        withSemaphore(lambda);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void arduinoOtaStart()
{
    FastLED.clear(true);
}

void arduinoOta()
{
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA
        .onStart(arduinoOtaStart)
        .onProgress(arduinoOtaProgress)
        .onError(arduinoOtaError);

    ArduinoOTA.begin();

    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void arduinoOtaProgress(unsigned int progress, unsigned int total)
{
    int progressPercent = progress / (total / 100);
    Serial.printf("Progress: %u%%\r", progressPercent);
    wrd((String)progressPercent, CRGB::White);
    FastLED.show();
}

void arduinoOtaError(ota_error_t error)
{
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
        Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
        Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
        Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
        Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
        Serial.println("End Failed");
}

void pixelLoop(void *param)
{
    for (;;)
    {
        drawTime(CRGB(0, 128, 128));
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

void flashQtt()
{
    for (int i = 0; i < 4; i++)
    {
        FastLED.clear(true);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        wrd("que", CRGB::White);
        FastLED.show();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
};

bool mqttConnect()
{
    if (mqttClient.connected())
        return true;

    mqttClient.stop();

    while (!mqttClient.connect(mqttBroker, mqttPort))
    {
        Serial.print("MQTT connection failed! Error code = ");
        Serial.println(mqttClient.connectError());

        withSemaphore(&flashQtt);

        vTaskDelay(60000 / portTICK_PERIOD_MS);
    }

    Serial.println("You're connected to the MQTT broker!");

    mqttClient.onMessage(onMqttMessage);
    mqttClient.subscribe("frigate/events");

    return true;
}

void mqttLoop(void *param)
{
    if (!wifiConnected())
    {
        wifiSetup();
    }

    while (mqttConnect())
    {
        mqttClient.poll();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

template <Segment segment>
void drawSegment(DigitOffset digit, CRGB rgb)
{
    int iDigit = static_cast<int>(digit);
    int iSegment = static_cast<int>(segment);
    int start = iDigit + iSegment;

    for (int i = start; i < start + SEGMENT_PIXEL_COUNT; i++)
    {
        leds[i] = rgb;
    }
}

void drawDots(CRGB rgb)
{
    int digitOffsetDots = static_cast<int>(DigitOffset::Dots);
    for (int i = digitOffsetDots; i < digitOffsetDots + DOTS_COUNT; i++)
    {
        leds[i] = CRGB(rgb);
    }
}

// A b c d E F g h I j L n o P q r S t u Y
const WordSegment segA = {"045689abefghlpqsty", &drawSegment<Segment::A>};
const WordSegment segB = {"02356789aefgjpqs", &drawSegment<Segment::B>};
const WordSegment segC = {"01234789adgipqy", &drawSegment<Segment::C>};
const WordSegment segD = {"013456789abdghijnoqsuy", &drawSegment<Segment::D>};
const WordSegment segE = {"023568bcdegjlostu", &drawSegment<Segment::E>};
const WordSegment segF = {"0268abcdefhlnoprtu", &drawSegment<Segment::F>};
const WordSegment segG = {"2345689abcdefghnopqrsty", &drawSegment<Segment::G>};

const WordSegment wordSegments[] = {segA, segB, segC, segD, segE, segF, segG};

void wrd(String wrd, CRGB rgb)
{
    if (wrd.length() > 4)
        return; // TODO: create a scrolling word render

    for (int i = 0; i < wrd.length(); i++)
    {
        char c = wrd[i];
        auto digit = DIGIT_OFFSETS[i];

        for (int j = 0; j < SEGMENT_PER_DIGIT; j++)
        {
            WordSegment curSegment = wordSegments[j];

            if (curSegment.c.indexOf(std::tolower(c)) > -1)
            {
                curSegment.segmentFunction(digit, rgb);
            }
            else
            {
                curSegment.segmentFunction(digit, CRGB::Black);
            }
        }
    }
}

String formatTime()
{
    
    int hours = timeinfo.tm_hour;
    int minutes = timeinfo.tm_min;

    if (hours > 12)
        hours %= 12;

    if (hours == 0)
        hours = 12;

    String sHours = hours >= 10 ? hours : " " + (String)hours;
    String sMinutes = minutes >= 10 ? minutes : "0" + (String)minutes;

    return sHours + sMinutes;
}

String getTime()
{
     if(getLocalTime(&timeinfo))
        return formatTime();

    return "set";
}

void drawTime(CRGB rgb)
{
    auto lambda = [&]()
    {
        wrd(getTime(), rgb);
        drawDots(timeinfo.tm_sec % 2 == 0 ? rgb : CRGB::Black);
        FastLED.show();
    };

    withSemaphore(lambda);
}

bool isDoorMessage(int length)
{
    const size_t capacity = 4 * JSON_ARRAY_SIZE(0) + 4 * JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(3) + 2 * JSON_OBJECT_SIZE(20) + length;
    DynamicJsonDocument doc(capacity);

    String json = mqttClient.readString();
    auto error = deserializeJson(doc, json);

    if (error)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        return false;
    }

    return doc["after"]["camera"] == "Front" && doc["type"] == "new";
}

void onMqttMessage(int messageSize)
{
    if (!isDoorMessage(messageSize))
        return;

    auto lambda = []()
    {
        for (int i = 0; i < 4; i++)
        {
            FastLED.clear(true);
            vTaskDelay(500 / portTICK_PERIOD_MS);
            wrd("door", CRGB::White);
            FastLED.show();
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    };

    withSemaphore(lambda);
}