#include <WiFi.h>
#include <ESP32Servo.h>
#include "esp_wifi.h"

/* ===============================
   Pin Configuration
   =============================== */

#define LED_PIN 2
#define SERVO_PIN 18
#define BUTTON_PIN 15

/* Proximity threshold (Adjust after testing) */
#define RSSI_THRESHOLD -44

/* WiFi Hotspot */
const char* ssid = "ESP32_HANDLEHERO";
const char* password = "12345678";

/* Servo */
Servo doorServo;

/* System State */
bool doorOpened = false;
bool paired = false;

/* Button State */
int prev_button_state = HIGH;
int button_state;

/* ===============================
   Flash LED Function
   =============================== */

void flashLED(int times){

    for(int i=0;i<times;i++){
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}

/* ================================
   Deep Sleep Function
================================ */

void goDeepSleep(){

    Serial.println("Entering deep sleep...");

    delay(1000);

    /* Wake up using button GPIO15 */
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, LOW);
    
    esp_deep_sleep_start();}


/* ===============================
   Setup
   =============================== */

void setup(){

    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    doorServo.attach(SERVO_PIN);
    doorServo.write(0);   // Door closed position

    /* Start WiFi Access Point */
    WiFi.softAP(ssid, password);

    Serial.println("Hands-Free Entry System Started");
    Serial.println(WiFi.softAPIP());
}

/* ===============================
   Loop
   =============================== */

void loop(){

    /* ===============================
       Pairing Detection
       =============================== */

    if(WiFi.softAPgetStationNum() > 0){
        paired = true;
    }

    /* ===============================
       Proximity Detection
       =============================== */

    if(paired){

        int rssi = -100;

        wifi_sta_list_t stationList;

        esp_wifi_ap_get_sta_list(&stationList);

        if(stationList.num > 0){
            rssi = stationList.sta[0].rssi;
        }

        Serial.print("RSSI: ");
        Serial.println(rssi);

        /* Open door if within range */
        if(rssi > RSSI_THRESHOLD){

            if(!doorOpened){

                Serial.println("Within range → Opening door");

                digitalWrite(LED_PIN, HIGH);

                doorServo.write(150);   // Fast response open

                flashLED(3);

                doorOpened = true;
            }
        }
    }

    /* ===============================
       Button Close Control
       =============================== */

    if(doorOpened){

        button_state = digitalRead(BUTTON_PIN);

        if(prev_button_state == LOW && button_state == HIGH){

            Serial.println("Button pressed → Closing door");

            doorServo.write(0);

            doorOpened = false;

            flashLED(1);

            goDeepSleep();
        }

        prev_button_state = button_state;
    }

    delay(1000);
}


