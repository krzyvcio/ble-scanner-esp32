#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>
#include <ArduinoJson.h>


#include <WiFiMulti.h>
#include <NTPClient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#define LED_BUILTIN 2

// Maksymalna liczba urządzeń, które można zeskanować
const int MAX_DEVICES = 50;

// Stałe JSON
const char *JSON_NAME = "name";
const char *JSON_ADDRESS = "address";
const char *JSON_RSSI = "rssi";

// Utworzenie obiektu JSON
DynamicJsonDocument devicesDoc(JSON_OBJECT_SIZE(MAX_DEVICES) + MAX_DEVICES * JSON_OBJECT_SIZE(13));

int scanTime = 15; // Czas skanowania w sekundach
BLEScan *pBLEScan;

// Dane do połączenia z serwerem czasu rzeczywistego
const int timeZone = 2;
const long utcOffsetInSeconds = timeZone * 3600; // Przesunięcie czasu UTC w sekundach
const char *ntpServer = "0.pl.pool.ntp.org";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, utcOffsetInSeconds);

// Dane do połączenia z siecią Telegram
#define BOTtoken "5831606623:AAGxSkxXdHeHM-gSYWBbue2BurW1EM6m5KE"
#define CHAT_ID "-1001980783840"
// Tworzenie obiektu klienta Telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Dane do połączenia z sieciami WiFi
WiFiMulti wifiMulti;
const char *ssid[] = {"UPC1044392", "Jakie haslo?", "Oaza"};
const char *password[] = {"vXe36aKrvket", "niepamietam", "twojamatka"};
const int num_wifi = sizeof(ssid) / sizeof(ssid[0]);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {

    // create JSON object
    JsonObject device = devicesDoc.createNestedObject();
    device["name"] = advertisedDevice.getName();
    device["address"] = advertisedDevice.getAddress().toString();
    device["rssi"] = advertisedDevice.getRSSI();
    device["payloadLength"] = advertisedDevice.getPayloadLength();
    device["payload"] = advertisedDevice.getPayload();
    device["manufacturer"] = advertisedDevice.getManufacturerData();
    device["serviceUUID"] = advertisedDevice.getServiceUUID().toString();
    device["serviceData"] = advertisedDevice.getServiceData();
    device["txPower"] = advertisedDevice.getTXPower();
    device["advertising"] = advertisedDevice.isAdvertisingService(BLEUUID((uint16_t)0x180F));
    device["txPower"] = advertisedDevice.getTXPower();
    device["advertising"] = advertisedDevice.isAdvertisingService(BLEUUID((uint16_t)0x180F));
    device["addressType"] = advertisedDevice.getAddressType();

    // strigify json
    String json;
    String jsonPretty;
    // serializeJson(devicesDoc, json);
    // print json with indent
    serializeJsonPretty(devicesDoc, jsonPretty);
    Serial.println(jsonPretty);
    bot.sendMessage(CHAT_ID, jsonPretty);

    // send to telegram
    // String url = "https://api.telegram.org/bot" + String(BOTtoken) + "/sendMessage?chat_id=" + String(CHAT_ID) + "&text=" + jsonPretty;
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
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
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
  Serial.println("Skanowanie...");
  // set led high
  digitalWrite(LED_BUILTIN, HIGH);
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.printf("Znaleziono urządzeń: %d\n", foundDevices.getCount());
  pBLEScan->clearResults();
  // set led low
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}