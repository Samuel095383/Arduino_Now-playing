#include <WiFiS3.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>

#include "secret_code.h"

// ===== DISPLAY CONFIG =====
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// ===== WIFI CONFIG =====
char* ssid = wifissid;
char* password = wifipass;

// ===== NAVIDROME CONFIG =====
char* serverAddress = "IP";
int port = 4533;
char* user = "USERNAME";
char* pass = "PASSWORD";

// ===== NETWORK OBJECTS =====
WiFiClient wifiClient;
HttpClient http(wifiClient, serverAddress, port);

void setup() {
  Serial.begin(9600);
  // DISPLAY SETTINGS
  lcd.begin(16, 2);
  // WIFI SETTINGS
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("CONNECTING...");
  }
  Serial.println("\nConnesso al WiFi!");
  Serial.print("Indirizzo IP Arduino: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    // URL FOR GETTING THE NOW PLAYING
    String request = "/rest/getNowPlaying.view?u=" + String(user) +
                     "&p=" + String(pass) +
                     "&v=1.16.1&c=nowplayingproject&f=json";

    // TRING TO GET THE REQUEST
    http.get(request);
    int statusCode = http.responseStatusCode();
    String response = http.responseBody();

    Serial.print("HTTP Code: ");
    Serial.println(statusCode);

    if (statusCode == 200) {
      Serial.println("Risposta OK");

      //PARSING JSON
      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.print("Errore parsing JSON: ");
        Serial.println(error.c_str());
      } else {
        JsonVariant nowPlaying = doc["subsonic-response"]["nowPlaying"]["entry"];

        if (nowPlaying.isNull()) {
          lcd.print("Nothing to play");
          
        } else {
          String title;
          String artist;

          if (nowPlaying.is<JsonArray>()) {
            JsonObject entry = nowPlaying[0];
            title = entry["title"] | "Sconosciuto";
            artist = entry["artist"] | "Sconosciuto";
          } else if (nowPlaying.is<JsonObject>()) {
            JsonObject entry = nowPlaying.as<JsonObject>();
            title = entry["title"] | "Sconosciuto";
            artist = entry["artist"] | "Sconosciuto";
          }
          lcd.setCursor(0, 0);
          lcd.print(title);
        }
      }
    } else {
      Serial.print("Errore HTTP: ");
      Serial.println(statusCode);
    }

    http.stop();
  }
}
