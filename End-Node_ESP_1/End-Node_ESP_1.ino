/*  
Code for END NODE ESP1 
This ESP will SEND Temprature to Coordinator ESP using ESP-NOW
Also it will RECIEVE data from Coordinator ESP using ESP-NOW

*  Developed by  Jay Joshi 
*  github.com/JayJoshi16

*/
#include <esp_now.h>
#include <WiFi.h>
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; //Coordinator ESP MAC address

#include <DHT.h>
#define DHT11PIN 5                  // DHT11 on GPIO 5
DHT dht(DHT11PIN, DHT11);

#include <ArduinoJson.h>
String recv_jsondata;
String send_jsondata;
StaticJsonDocument<256> doc_to_espnow;
StaticJsonDocument<256> doc_from_espnow;  

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  
  dht.begin();
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);            
  
 esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
                                                  // Sending Tempreature Every 4 Seconds
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  doc_to_espnow["v1"] = temp;                       // Creating JSON data. Here { v1 : 28.55 }
  doc_to_espnow["v2"] = hum;                       // Creating JSON data. Here { v2 : 34.35 }
  serializeJson(doc_to_espnow, send_jsondata);
  esp_now_send(broadcastAddress, (uint8_t *) send_jsondata.c_str(), send_jsondata.length());
                                                    // Sending it to Coordinater ESP
  Serial.println(send_jsondata); 
  send_jsondata = "";
  delay(4000);
}
