#include <WiFiClient.h>
#include <HTTPClient.h>

void http_request(String url, int & responseCode, String & responseString, String header[] = nullptr) {

  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(url);

  if (header != nullptr) {
    http.addHeader(header[0], header[1]);
  }

  // Send HTTP GET request
  responseCode = http.GET();

  if (responseCode > 0) {
    responseString = http.getString();
  }

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