/*

  Coordinator ESP's Code.
  This ESP will recieve ESPNOW data from END Node ESP's and Serially trasnfers it to BLYNK ESP and vise versa.
  MAC Address of this ESP is Provided to ESD Node ESP.

    Developed by  Jay Joshi
    github.com/JayJoshi16

    Modified by Sachin Soni

    Developed for techiesms
    https://www.youtube.com/techiesms

    Modified by STM to work
    load this code onto ESPNOW board

*/

#include <esp_now.h>
#include <WiFi.h>
#include <SimpleTimer.h> //https://github.com/jfturcot/SimpleTimer
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <SPI.h>
#include <Wire.h>                     // oled
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// There must be one global SimpleTimer object.
// More SimpleTimer objects can be created and run,
// although there is little point in doing so.
SimpleTimer timer;


uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Universal Broadcast address


StaticJsonDocument<256> doc_from_espnow; // JSON Doc for Receiving data from ESPNOW Devices
StaticJsonDocument<256> doc_to_espnow; // JSON Doc for Transmitting data to ESPNOW Devices

String recv_jsondata;               // recieved JSON string

int temperature = 0;
int humidity = 0;

// Hardware Serial 2
#define RXD2 16
#define TXD2 17

// OLED screen Configurations
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);



// ESPNOW Send Callback Function
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//ESPNOW Receive Callback Function
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{

  char* buff = (char*) incomingData;
  recv_jsondata = String(buff);
  Serial.print("Recieved from ESPNOW: "); Serial.println(recv_jsondata);
  DeserializationError error = deserializeJson(doc_from_espnow, recv_jsondata);
  if (!error) {
    Serial.print("Serilising to Serial2: ");
    Serial.println(recv_jsondata);
    temperature  = doc_from_espnow["v1"];                 // Storing Temperature Data
    humidity  = doc_from_espnow["v2"];                 // Storing Humidity Data
    serializeJson(doc_from_espnow, Serial2);            // Writing Data to Serial2
  }

  else
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

}

// Function that Displays tempreature on OLED
void displayText()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 10);
  display.print("Temperature :");
  display.println(temperature);
  display.print("Humidity :");
  display.println(humidity);
  display.display();
}

void setup() {

  //ONBOARD LED WILL GLOW IN CASE OF RESET
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(2000);
  digitalWrite(2, LOW);
  delay(2000);

  // Initialising UART Communication
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // Initialising OLED (I2C) Communication
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(200);
  display.display();
  display.clearDisplay();
  delay(200);



  WiFi.mode(WIFI_STA);

  // Initialising ESPNOW Communication
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Defining Callback Functions
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Adding Peer Devices
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  timer.setTimeout(2000, displayText);                  // One Shot Timer to display data on OLED
}

void loop()
{
  if (Serial2.available())
  {
    // Recieving data (JSON) from BLYNK ESP
    String recv_str_jsondata = Serial2.readStringUntil('\n');

    //Serializing JSON
    serializeJson(doc_to_espnow, recv_str_jsondata);
    Serial.println(recv_str_jsondata);

    // Broadcasting data (JSON) via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) recv_str_jsondata.c_str(), sizeof(recv_str_jsondata) * recv_str_jsondata.length());

    if (result == ESP_OK)
    {
      Serial.println("Sent with success");
    }
    else
    {
      Serial.println(result);
    }
    delay(20);
  }
}
