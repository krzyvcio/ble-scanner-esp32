#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>
#include <ArduinoJson.h>

#include <WiFiMulti.h>
#include <NTPClient.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>

DNSServer dnsServer;
AsyncWebServer server(80);

#define LED_BUILTIN 2

// Maksymalna liczba urządzeń, które można zeskanować
const int MAX_DEVICES = 50;

// Stałe JSON
const char *JSON_NAME = "name";
const char *JSON_ADDRESS = "address";
const char *JSON_RSSI = "rssi";

// Utworzenie obiektu JSON
DynamicJsonDocument devicesDoc(JSON_OBJECT_SIZE(MAX_DEVICES) + MAX_DEVICES * JSON_OBJECT_SIZE(13));

int scanTime = 4;

BLEScan *pBLEScan;

// Dane do połączenia z serwerem czasu rzeczywistego
const int timeZone = 2;
const long utcOffsetInSeconds = timeZone * 3600; // Przesunięcie czasu UTC w sekundach
const char *ntpServer = "0.pl.pool.ntp.org";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);

// Dane do połączenia z sieciami WiFi
WiFiMulti wifiMulti;
const char *ssid[] = {"Krownice", "Pustynia"};
const char *password[] = {"1q2w3e4r5t6y7u8", "1q2w3e4r5t6y7u88"};
const int num_wifi = sizeof(ssid) / sizeof(ssid[0]);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    // Create a new JSON object for each device
    DynamicJsonDocument deviceDoc(JSON_OBJECT_SIZE(13));
    JsonObject device = deviceDoc.to<JsonObject>();

    device["name"] = advertisedDevice.getName();
    device["address"] = advertisedDevice.getAddress().toString();
    device["rssi"] = advertisedDevice.getRSSI();
    device["payloadLength"] = advertisedDevice.getPayloadLength();
    // device["payload"] = advertisedDevice.getPayload();
    // device["manufacturer"] = advertisedDevice.getManufacturerData();
    device["serviceUUID"] = advertisedDevice.getServiceUUID().toString();
    // device["serviceData"] = advertisedDevice.getServiceData();
    device["txPower"] = advertisedDevice.getTXPower();
    device["advertising"] = advertisedDevice.isAdvertisingService(BLEUUID((uint16_t)0x180F));
    // device["addressType"] = advertisedDevice.getAddressType();

    // Serialize the JSON object
    String json;
    serializeJson(device, json);
    Serial.println(json);
    Serial.println("\n");

    DynamicJsonDocument doc(1024);
    doc["device"] = device;

    HTTPClient http;

    http.begin("http://192.168.100.95");                // Specify the URL
    http.addHeader("Content-Type", "application/json"); // Specify content-type header
    String requestBody;
    serializeJson(doc, requestBody); // Convert JSON object to String

    int httpResponseCode = http.POST(requestBody); // Send the actual POST request

    if (httpResponseCode > 0)
    {
      String response = http.getString(); // Get the response to the request
      Serial.println(httpResponseCode);   // Print return code
      // Serial.println(response);           // Print request answer
    }
    else
    {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  }
};

void setup()
{
  // pin mode led
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("ESP32 BLE Scanner");

  // Dodawanie sieci WiFi do listy sieci
  Serial.println("Connecting to WiFi...");
  for (int i = 0; i < num_wifi; i++)
  {
    wifiMulti.addAP(ssid[i], password[i]);
  }
  // connect wifi
  Serial.println("Connecting Wifi...");
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print(".");
    // blink led
    digitalWrite(LED_BUILTIN, LOW);
    delay(75);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(75);
  }
  Serial.println("\nConnected to Wifi!");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(125);
  pBLEScan->setWindow(65);
}

void loop()
{
  Serial.println("Scanning...");
  digitalWrite(LED_BUILTIN, HIGH);
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);

  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  digitalWrite(LED_BUILTIN, LOW);
  pBLEScan->clearResults();
  // set led low

  delay(1000);
}