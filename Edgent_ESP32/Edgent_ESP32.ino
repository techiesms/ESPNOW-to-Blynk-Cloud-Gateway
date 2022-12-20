/*

  BLYNK ESP's Code.

  Paste this code in EXAMPLES > BLYNK > Blynk_edgent > Edgent_ESP32

  This ESP will recieve JSON data from Coordinator ESP's and Sends it to BLYNK Cloud and vise versa.
  Only this ESP is connected to Internet

    Developed by  Jay Joshi
    github.com/JayJoshi16

  Modified by Sachin Soni

  developed for techiesms

  Modified to work by STM
  add this code to the Blynk WiFi ESP32 board

*/
//#define BLYNK_TEMPLATE_ID "TMPLkMNvOYYR"
//#define BLYNK_DEVICE_NAME "ESPNOW"


#define BLYNK_TEMPLATE_ID "TMPLklmoR0_L"
#define BLYNK_DEVICE_NAME "ESPNOW"
//#define BLYNK_AUTH_TOKEN "AjNNyentXYnGjtKxM4-YEl1Y8fQ96LHY"

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "Mycastle";  //Enter your WIFI Name  // STM added
//char pass[] = "chance1845fence";  //Enter your WIFI Password  // STM added

#define BLYNK_FIRMWARE_VERSION        "0.1.0"
#define BLYNK_PRINT Serial
#define APP_DEBUG
#define USE_WROVER_BOARD

#include "BlynkEdgent.h"
#include <ArduinoJson.h>

String recv_str_jsondata;

StaticJsonDocument<256> doc_send;
StaticJsonDocument<256> doc_recv;

#define RXD2 16
#define TXD2 17


BLYNK_WRITE(V0)                                 // Read from Virtual pin V4
{

  bool s0 = param.asInt();                    // parameter as int
  if (s0 == HIGH)
  {
    doc_send["v0"] = "v0_on";                // Writing { "v4" : 1 } on Serial2
    Serial.println("Sended v0 : v0_on");
    serializeJson(doc_send, Serial2);
    delay(10);
  }
  else if (s0 == LOW)
  {
    doc_send["v0"] = "v0_off";                // Writing { "v4" :0 } on Serial2
    Serial.println("Sended v0 : v0_off");
    serializeJson(doc_send, Serial2);
    delay(10);
  }
}

void setup()
{
  BlynkEdgent.begin();

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);        //Hardware Serial of ESP32

}

void loop() {

  BlynkEdgent.run();
  
  // Recieving data (JSON) from Coordinator ESP
  if (Serial2.available())
  {

    recv_str_jsondata = Serial2.readStringUntil('\n');
    Serial.println(recv_str_jsondata);
    DeserializationError error = deserializeJson(doc_recv, recv_str_jsondata);

    if (!error) {                                           // if not error in deserialization
      float temp = doc_recv["v1"];                       // fetch temprature data from JSON . Here { v1 : 28.55 }
      float hum = doc_recv["v2"];                       // fetch temprature data from JSON . Here { v2 :32.25 }

      if (temp > 0) {
        Blynk.virtualWrite(V1, temp);                     // writing temprature to BLYNK Cloud
        Serial.print("temp ="); Serial.println(temp);
      }
      if (hum > 0) {
        Blynk.virtualWrite(V2, hum);                     // writing temprature to BLYNK Cloud
        Serial.print("hum ="); Serial.println(hum);
      }

    }

    else {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    recv_str_jsondata = "";
  }

}
