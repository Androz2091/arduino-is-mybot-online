#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

#include "secrets.h"

ESP8266WiFiMulti WiFiMulti;

using namespace websockets;
WebsocketsClient client;

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;
const char token[] = SECRET_TOKEN;
const char watched_user_id[] = WATCHED_USER_ID;

const uint8_t RED_LED = D1;
const uint8_t GREEN_LED = D0;

bool connected = false;
int lastHeartbeatTime = -1;
int heartbeatInterval = 60000;
int sequence = -1;

bool offline = false;

void onMessageCallback(WebsocketsMessage message) {
    DynamicJsonDocument payloadObj(2048);
    deserializeJson(payloadObj, message.data());
    if(payloadObj["s"]){
        sequence = payloadObj["s"];
    }
    if(payloadObj["op"] == 10){
        heartbeatInterval = payloadObj["d"]["heartbeat_interval"];
        login();
    }
    if(payloadObj["t"] == "PRESENCE_UPDATE"){
        if(payloadObj["d"]["user"]["id"] == watched_user_id){
            const String newStatus = payloadObj["d"]["status"];
            Serial.println("Target became "+newStatus);
            if(newStatus == "offline"){
                offline = true;
            } else {
                offline = false;
            }
        }
    }
}

void login(){
    Serial.println("Logging in...");
    DynamicJsonDocument payloadObj(1024);
    payloadObj["op"] = 2;
    JsonObject data = payloadObj.createNestedObject("d");
    data["token"] = token;
    data["intents"] = 256;
    JsonObject properties = data.createNestedObject("properties");
    properties["$os"] = "darwin";
    properties["$browser"] = "discly";
    properties["$device"] = "discly";
    String payload = "";
    serializeJson(payloadObj, payload);
    Serial.println(payload);
    client.send(payload);
    Serial.println("Login payload sent!");
}

void sendHeartBeat(){
    Serial.println("Sending heartbeat...");
    DynamicJsonDocument payloadObj(1024);
    payloadObj["op"] = 1;
    payloadObj["d"] = sequence;
    String payload = "";
    serializeJson(payloadObj, payload);
    client.send(payload);
    lastHeartbeatTime = millis();
    Serial.println("Heartbeat payload sent!");
}

void onEventsCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("Websocket Connnection Opened!");
        Serial.println(data);
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("Websocket Connnection Closed!");
        Serial.println(data);
    }
}

void setup() {

    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);

    Serial.begin(9600);

    Serial.println();
    Serial.println();
    Serial.println();

    for (uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    connectToWifi();

    // Setup Callbacks
    client.onMessage(onMessageCallback);
    client.onEvent(onEventsCallback);

}

void connectToWifi(){
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssid, pass);
}

void loop() {
    Serial.println();
    if (int stat = WiFiMulti.run() == WL_CONNECTED && !connected) {
      Serial.println("Connected to WiFi network");
      // Connect to server
      client.connect("wss://gateway.discord.gg/?v=6&encoding=json");
      connected = true;
    }
 
    if(offline){
        if(digitalRead(RED_LED) == HIGH){
            digitalWrite(RED_LED, LOW);
            delay(500);
        }
        if(digitalRead(GREEN_LED) == HIGH){
            digitalWrite(GREEN_LED, LOW);
        }
        digitalWrite(RED_LED, HIGH);
        delay(500);
        digitalWrite(RED_LED, LOW);
        delay(500);
    } else {
        digitalWrite(GREEN_LED, HIGH);
    }
    delay(1000);
    client.poll();
    if (millis() - lastHeartbeatTime >= heartbeatInterval) {
        sendHeartBeat();
    }
}
