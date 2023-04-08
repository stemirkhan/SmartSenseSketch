#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_Sensor.h>
#include <WiFiClientSecureBearSSL.h>


#define BMP_CS   15


Adafruit_BMP280 bmp(BMP_CS);
Adafruit_AHTX0 aht;

const char* ssid = "";
const char* password = "";
const char* server_host = "";
const char* server_port = "";
const char* access_token = "";

void setup(){
  if (!bmp.begin()) {
    while (1) delay(10);
    
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  aht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop(){
  int sensorValue = analogRead(A0); 
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

  if (WiFi.status() == WL_CONNECTED) {

    String url = "https://" + server_host + ":" + server_port + "/#" + "access_token=" + access_token + "&temperature=" + bmp.readTemperature() + "&pressure=" + bmp.readPressure() + "&carbonMonoxide=" +  sensorValue + "&humidity=" + humidity.unitless_percent;
    HTTPClient https;

    https.begin(*client, url);  
    int httpCode = https.GET();

    https.end();
  }

  del