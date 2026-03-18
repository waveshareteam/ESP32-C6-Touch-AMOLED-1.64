#include <WiFi.h>

const char* ssid = "WSTEST";          // Change to your WiFi name
const char* password = "waveshare0755";         // Change to your WiFi password
void setup()
{
  WiFi.mode(WIFI_STA); // Set to STA mode
  WiFi.begin(ssid, password);
  
  printf("Connecting to WiFi\n");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  printf("IP Address: %s\n",WiFi.localIP().toString().c_str());
}
void loop()
{
  // Your code
}
