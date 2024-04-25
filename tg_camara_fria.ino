#include <WiFi.h>
#include <OneWire.h>
#include <ArduinoJson.h>
#include "EspMQTTClient.h"
#include <WiFiClientSecure.h>
#include <DallasTemperature.h>
#include <UniversalTelegramBot.h>

#define ONE_WIRE_BUS 4
#define BOTtoken "7087399197:AAHeK6iJnIhPrnkV0pvoHF7LyhubLj1_rbA"
#define CHAT_ID "-4109185645"

const char *ssid = "CAMARAFRIA";
const char *password = "camarafria";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClientSecure wifiClient;
UniversalTelegramBot bot(BOTtoken, wifiClient);

char temp[100];

EspMQTTClient client(
    ssid,
    password,
    "mqtt.tago.io",                         // MQTT Broker server ip padrão da tago
    "Default",                              // username
    "1b548e2f-0ea0-48e2-9bf0-5f5bf134fcc2", // Código do Token
    "TestClient",                           // Client name that uniquely identify your device
    1883                                    // The MQTT port, default to 1883. this line can be omitted
);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    wifiClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }

    sensors.begin();

    Serial.println(WiFi.localIP());
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished() {}

// loop do programa
void loop()
{
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);

    StaticJsonDocument<300> TEMP;

    TEMP["variable"] = "temperature";
    TEMP["value"] = temperature;
    TEMP["unit"] = "C";

    serializeJson(TEMP, temp);

    float maxTemp = 33;
    float minTemp = 25;

    if (temperature >= maxTemp || temperature <= minTemp)
    {
        String mensagem = "\xE2\x9A\xA0 Temperatura está fora dos parâmetros desejados \xE2\x9A\xA0\n\n";
        mensagem += "\xF0\x9F\x8C\xA1 Temperatura atual: ";
        mensagem += temperature;
        mensagem += " °C";

        bot.sendMessage(CHAT_ID, mensagem, "");
    }

    client.publish("temperature", temp); // You can activate the retain flag by setting the third parameter to true

    delay(10000);

    client.loop();
}