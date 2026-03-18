#include <WiFi.h>
#include "esp_wifi.h"
#include "esp_netif.h"

const char *ssid = "WSTEST";
const char *password = "waveshare0755";

void setup() {
    delay(2000);
    // Set ESP32 to AP mode
    WiFi.softAP(ssid, password);
    printf("WiFi AP Started\n");
    
    // Print the IP address of the AP
    printf("AP IP Address: %s\n",WiFi.softAPIP().toString().c_str());
}

void loop() {
    delay(5000); // Check connected devices every 5 seconds
    printConnectedDevices();
}

void printConnectedDevices() {
    wifi_sta_list_t stationList;
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

    // Get the list of connected devices
    esp_wifi_ap_get_sta_list(&stationList);
    
    printf("Connected Devices: %d\n",stationList.num);

    if (netif == nullptr) {
        printf("Failed to get AP interface!\n");
        return;
    }

    for (int i = 0; i < stationList.num; i++) {
        printf("Device %d MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", i + 1,
                      stationList.sta[i].mac[0], stationList.sta[i].mac[1], stationList.sta[i].mac[2],
                      stationList.sta[i].mac[3], stationList.sta[i].mac[4], stationList.sta[i].mac[5]);
        
        // Get the device's IP address (ESP32 in AP mode cannot directly get the client's IP)
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            printf("Device %d IP: %s\n", i + 1, ip4addr_ntoa((const ip4_addr_t*)&ip_info.ip));
        }
    }
}
