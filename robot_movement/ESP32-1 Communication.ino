// MQTT Set-up
#include "Arduino.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"
#include <HTTPClient.h>

const char *ssid = "Palapaki";
const char *pass = "catsarethebest";

// Test Mosquitto server, see: https://test.mosquitto.org
char *server = "mqtt://192.168.204.140:1883";

String command = "DONTSTART";
char *subscribeTopic = "start_A/#";
char *publishTopic = "parked/ready";
String url = "http://192.168.204.182:5001/api/addorder";

ESP32MQTTClient mqttClient; // all params are set later


// ESP32-1 Pin definitions
#define ACTION_A_TRIGGER_PIN 16    // Pin to signal ESP32-2 to start Action A
#define ACTION_A_RESPONSE_PIN 17   // Pin to receive completion of Action A from ESP32-2
#define ACTION_B_COMPLETE_PIN 18    // Pin to signal ESP32-2 that Action B is complete
#define ACTION_C_RESPONSE_PIN 19   // Pin to receive completion of Action C from ESP32-2
#define SWITCH 4                // Button to toggle Action B on/off
#define CHICKEN 14 // 0
#define FISH 27 // 1
#define SALAD 26 // 2
#define ACK 13

int volatile chicken_count = 0;
int volatile fish_count = 0;
int volatile salad_count = 0;
// int prev_chick_state = 0;
// int prev_fish_state = 0;
// int prev_sal_state = 0;

#define debouncetime 200 
volatile bool pressed = false; 
volatile unsigned long lastdebouncetime = 0; 

void IRAM_ATTR isr() { 
  if ((millis() - lastdebouncetime) > debouncetime) {
    pressed = true;
    lastdebouncetime = millis();
  }
}


void setup() {
  Serial.begin(115200);
  delay(3000);

  // MQTT Set-up
  mqttClient.enableDebuggingMessages();

  mqttClient.setURI(server);
  mqttClient.enableLastWillMessage("lwt", "I am going offline");
  mqttClient.setKeepAlive(30);
  WiFi.begin(ssid, pass);
  WiFi.setHostname("c3test");
  delay(3000);
  mqttClient.loopStart();

  // Set up pins
  pinMode(ACTION_A_TRIGGER_PIN, OUTPUT);
  pinMode(ACTION_A_RESPONSE_PIN, INPUT);
  pinMode(ACTION_B_COMPLETE_PIN, OUTPUT);
  pinMode(ACTION_C_RESPONSE_PIN, INPUT);
  pinMode(CHICKEN, INPUT);
  pinMode(FISH, INPUT);
  pinMode(SALAD, INPUT);
  pinMode(SWITCH, INPUT_PULLUP);
  pinMode(ACK, OUTPUT);

  attachInterrupt(SWITCH, isr, FALLING); 

  // Initialize trigger pins to LOW
  digitalWrite(ACK, LOW);
  digitalWrite(ACTION_A_TRIGGER_PIN, LOW);
  digitalWrite(ACTION_B_COMPLETE_PIN, LOW);
  digitalWrite(ACTION_C_RESPONSE_PIN, LOW);
  // digitalWrite(CHICKEN, LOW);
  // digitalWrite(FISH, LOW);
  // digitalWrite(SALAD, LOW);


  Serial.println("ESP32-1 waiting for START command...");
}

void loop() {
  // String command = Serial.readStringUntil('\n');
  if (command == "START") {
    Serial.println("START command received. Triggering Action A on ESP32-2...");
    digitalWrite(ACTION_A_TRIGGER_PIN, HIGH);  // Signal ESP32-2 to start Action A
    delay(1000);

    // Wait for ESP32-2 to complete Action A
    while (digitalRead(ACTION_A_RESPONSE_PIN) == LOW) {
      Serial.println("Waiting...");
      delay(1000);
    }
    Serial.println("ESP32-2 has completed Action A. Starting Action B...");

    // Execute Action B with button control
    performActionB();

    Serial.println("Waiting for Action C from ESP32-2...");
    // Wait for ESP32-2 to complete Action C by pulling the pin HIGH
    while (digitalRead(ACTION_C_RESPONSE_PIN) == LOW) {
      Serial.println("Waiting...");
      delay(1000);
    }

    Serial.println("ESP32-2 has completed Action C. Resetting for next START command.");
    reset();
  }
}

void performActionB() {
  Serial.println("Performing Action B...");
  // digitalWrite(CHICKEN, LOW);
  // digitalWrite(FISH, LOW);
  // digitalWrite(SALAD, LOW);
  while (true) {
    // int chick_state = digitalRead(CHICKEN);
    // int fish_state = digitalRead(FISH);
    // int sal_state = digitalRead(SALAD);
    if ( digitalRead(CHICKEN) == HIGH) {
      chicken_count=1;
      digitalWrite(ACK, HIGH);
      delay(1000);
      digitalWrite(ACK, LOW);
      // digitalWrite(CHICKEN, LOW);
      Serial.println("Chicken Count: ");
      Serial.println(chicken_count);
      pressed=true;
    }
    else if (digitalRead(FISH) == HIGH) {
      fish_count=1;
      digitalWrite(ACK, HIGH);
      delay(1000);
      digitalWrite(ACK, LOW);
      // digitalWrite(FISH, LOW);
      Serial.println("Fish Count: ");
      Serial.println(fish_count);
      pressed=true;
    }
    else if (digitalRead(SALAD) == HIGH) {
      salad_count=1;
      digitalWrite(ACK, HIGH);
      delay(1000);
      digitalWrite(ACK, LOW);
      // digitalWrite(SALAD, LOW);
      Serial.println("Salad Count: ");
      Serial.println(salad_count);
      pressed=true;
    }
    
    // Check if action is completed
    if (pressed) {
      Serial.println("Action B completed.");
      digitalWrite(ACTION_B_COMPLETE_PIN, HIGH);  // Signal ESP32-2 that Action B is complete
      break;
    }
  }
  
  // Send HTTP Request
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String newUrl = url + "?chickenOrder=" + String(chicken_count) + 
                        "&fishOrder=" + String(fish_count) + 
                        "&saladOrder=" + String(salad_count);
      Serial.println(newUrl);
      http.begin(newUrl);  // Specify request destination
      // http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Specify content-type header

      // Convert integers to String for concatenation
      // String postData = "chickenOrder=" + String(chicken_count) + 
      //                   "&fishOrder=" + String(fish_count) + 
      //                   "&saladOrder=" + String(salad_count);

      int httpCode = http.GET();  // Send POST request with the data
      String payload = http.getString();  // Get the response payload

      Serial.println(httpCode);  // Print HTTP return code
      Serial.println(payload);  // Print request response payload

      http.end();  // Close connection
  } else {
      Serial.println("WiFi not connected");
  }
}


void reset(){
  digitalWrite(ACTION_A_TRIGGER_PIN, LOW);      // Reset signal to ESP32-2
  digitalWrite(ACTION_B_COMPLETE_PIN, LOW);     // Reset signal to ESP32-2
  digitalWrite(ACTION_C_RESPONSE_PIN, LOW);

  int volatile chicken_count = 0;
  int volatile fish_count = 0;
  int volatile salad_count = 0;
  // Tell esp32cam to start detecting hand again
  String msg = "Parked";
  mqttClient.publish(publishTopic, msg, 0, false);
  // Reset push button
  pressed = false;
  // Reset main loop
  command = "DONTSTART";
  Serial.println("ESP32-1 waiting for START command...");
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isMyTurn(client)) // can be omitted if only one client
    {
        mqttClient.subscribe(subscribeTopic, [](const String &topic, const String &payload) 
                            {if(payload == "START"){command = "START";} });

    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}