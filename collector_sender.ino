#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_Sensor.h>
#include <WiFiClientSecureBearSSL.h>
#include <Ticker.h>

Ticker ticker_buzzer;
bool state_timer_buzzer = false;

Adafruit_BMP280 bmp;
Adafruit_AHTX0 aht;
const int Buzzer = D7; 

const char* ssid = "";
const char* password = "";
const char* server_host = "";
const char* server_port = "";
const char* access_token = "";


void setup(){
  pinMode (Buzzer, OUTPUT);

  if (! aht.begin()) {
    while (1) delay(10);
  }

  if (!bmp.begin(0x76)) {
    while (1) delay(10);
  }

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

  if(sensorValue > 700 && !state_timer_buzzer){
    ticker_buzzer.attach(0.1, handler_activate_buzzer);
    state_timer_buzzer = true;
  }
  else if(sensorValue <= 700){
    ticker_buzzer.detach();
    digitalWrite (Buzzer, LOW);
    state_timer_buzzer = false;
  }

  send_data(bmp.readTemperature(), bmp.readPressure(), sensorValue, humidity.unitless_percent);
  delay(2000);
}


void handler_activate_buzzer(){
  digitalWrite (Buzzer, !(digitalRead(Buzzer)));
}


void send_data(double temperature, double pressure, double carbonMonoxide, double humidity){
 std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  client->setInsecure();

  if (WiFi.status() == WL_CONNECTED) {

    String url = "https://" +  String(server_host) + ":" + server_port + "/#" + "access_token=" + access_token + "&temperature=" + temperature + "&pressure=" + pressure * 0.75 + "&carbonMonoxide=" +  carbonMonoxide + "&humidity=" + humidity;
    HTTPClient https;

    https.begin(*client, url); 
    int httpCode = https.GET();

    Serial.println(httpCode);

    https.end();
  }
}