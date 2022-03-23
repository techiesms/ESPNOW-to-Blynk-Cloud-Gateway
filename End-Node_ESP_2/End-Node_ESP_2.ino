/*  
Code for END NODE ESP 2
This will RECIEVE data from Coordinator ESP using ESP-NOW and Toggle BuiltIn LED
*  Developed by  Jay Joshi 
*  github.com/JayJoshi16
*/


#include <esp_now.h>
#include <WiFi.h>
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
#include <ArduinoJson.h>

String recv_jsondata;
String send_jsondata;


StaticJsonDocument<256> doc_to_espnow;  
StaticJsonDocument<256> doc_from_espnow;  // for data < 1KB
//DynamicJsonDocument doc(1024);  // for data > 1KB


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  char* buff = (char*) incomingData;
  recv_jsondata = String(buff);
  Serial.print("Recieved "); Serial.println(recv_jsondata);
  DeserializationError error = deserializeJson(doc_from_espnow, recv_jsondata);
  if (!error) {
    String led_status   = doc_from_espnow["v5"]; //values are 1 or 0
    Serial.print("led status : ");Serial.println(led_status);
    if(led_status=="v0_on")
    digitalWrite(2,HIGH);
    else if(led_status=="v0_off")
    digitalWrite(2,LOW);
    else
    Serial.println(led_status);

  }

  else {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

}

void setup() {
  
  pinMode(2,OUTPUT);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
 
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
 
}
