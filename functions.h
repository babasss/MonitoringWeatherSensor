#include <WiFiClient.h>
//#include <WiFiClientSecure.h>
#include <HTTPClient.h>

void http_request(String url, int & responseCode, String & responseString, String header[] = nullptr) {

  HTTPClient http;
  //WiFiClientSecure client;
  //client.setInsecure();

  //HTTPClient https;

  Serial.println(url);

  // Your Domain name with URL path or IP address with path
  http.begin(url);
  //http.begin("https://www.timeapi.io/api/timezone/zone?timeZone=Europe/Paris");
  http.setTimeout(5000);
  //https.begin(client, "http://192.168.1.38/package_DFRobot_index.json");
  //https.setTimeout(5000);
  

  if (header != nullptr) {
    http.addHeader(header[0], header[1]);
  }

  // Send HTTP GET request
  responseCode = http.GET();

  if (responseCode > 0) {
    responseString = http.getString();
  }

  //Serial.println("DEBUG HTTP CODE : " + String(responseCode));
  //Serial.println("DEBUG HTTP STRING : " + responseString.substring(0, 100));

  // Free resources
  http.end();

}

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("#REVEIL : Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("#REVEIL : Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("#REVEIL : Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("#REVEIL : Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("#REVEIL : Wakeup caused by ULP program"); break;
    default:                        Serial.printf("#REVEIL : Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

template <typename T>
bool tryRefresh(const String& name, T& obj, bool (T::*refreshFunc)()) {
//bool tryRefresh(String name, bool (*refreshFunc)()) {
    for (int attempt = 1; attempt <= maxRetries; attempt++) {
        if ((obj.*refreshFunc)()) {
            Serial.print("> ");
            Serial.print(name);
            Serial.println(" : Données rafraîchies avec succès !");
            return true;
        } else {
            Serial.print("> ");
            Serial.print(name);
            Serial.print(" : Tentative ");
            Serial.print(attempt);
            Serial.println(" échouée, nouvel essai...");
        }
        delay(retryPause); // Pause de 5s entre chaque tentative
    }
    Serial.print("> ");
    Serial.print(name);
    Serial.println(" : Échec du rafraîchissement des données après 5 tentatives.");
    return false;
}


time_t convertToEpoch(const char* dateStr) {
  struct tm tm;
  memset(&tm, 0, sizeof(tm));

  sscanf(dateStr, "%d-%d-%d %d:%d:%d",
         &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
         &tm.tm_hour, &tm.tm_min, &tm.tm_sec);

  tm.tm_year -= 1900;  // struct tm attend années depuis 1900
  tm.tm_mon  -= 1;     // struct tm attend mois 0–11

  time_t epoch = mktime(&tm);

  //Serial.printf("%s - %ld \n", dateStr, (long)epoch);
  
  return epoch;
}