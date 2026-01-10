int wifi_signal; 

uint8_t StartWiFi() {
  Serial.print("\r\n> Connecting to: "); Serial.println(String(ssid));
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoReconnect(true);

  WiFi.begin(ssid, ssid_password);

  IPAddress local_IP(192, 168, 1, 253);
  IPAddress gateway(192, 168, 1, 254);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(1, 1, 1, 1);  // CloudFlare
  IPAddress secondaryDNS(1, 0, 0, 1); // CloudFlare

  //WiFi.config(local_IP.fromString(String(local_IP_String)), gateway.fromString(String(gateway_String)), subnet.fromString(String(subnet_String)), primaryDNS.fromString(String(primaryDNS_String)), secondaryDNS.fromString(String(secondaryDNS_String)));
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  
  unsigned long start = millis();
  uint8_t connectionStatus;

  bool AttemptConnection = true;
  while (AttemptConnection) {
    connectionStatus = WiFi.status();
    if (millis() > start + 15000) { // Wait 15-secs maximum
      AttemptConnection = false;
    }
    if (connectionStatus == WL_CONNECTED || connectionStatus == WL_CONNECT_FAILED) {
      AttemptConnection = false;
    }
    delay(50);
  }

  if (connectionStatus == WL_CONNECTED) {
    //wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
    //Serial.println("> WiFi connected at: " + WiFi.localIP().toString() + " with signal = " + String(wifi_signal));
    Serial.println("Wi-Fi Configuration:");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());  // SSID name of the connected network

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());  // Local IP address assigned by the router

    Serial.print("Subnet Mask: ");
    Serial.println(WiFi.subnetMask());  // Subnet Mask of the network

    Serial.print("Gateway IP: ");
    Serial.println(WiFi.gatewayIP());  // Gateway IP (usually your router's IP)

    Serial.print("DNS Server 1: ");
    Serial.println(WiFi.dnsIP(0));  // Primary DNS server IP

    Serial.print("DNS Server 2: ");
    Serial.println(WiFi.dnsIP(1));  // Secondary DNS server IP

    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());  // MAC address of the ESP32

    Serial.print("Signal Strength (RSSI): ");
    Serial.println(WiFi.RSSI());  // Wi-Fi signal strength in dBm
}
  else Serial.println("> WiFi connection *** FAILED ***");

  return connectionStatus;
}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("> WiFi disconnect");
}


