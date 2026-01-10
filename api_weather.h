class Weather {
public:
    JsonDocument docCurrent;
    JsonDocument docForecast;

    Weather() {}

    bool refresh() {
        int resCodeCurrent, resCodeForecast;
        String resStringCurrent, resStringForecast;
        char url[256];

        // --- 1. Météo Actuelle ---
        snprintf(url, sizeof(url), "%s/data/2.5/weather?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", 
                 weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        http_request(url, resCodeCurrent, resStringCurrent);
        
        if (resCodeCurrent > 0) {
            docCurrent.clear();
            deserializeJson(docCurrent, resStringCurrent);
            docCurrent.shrinkToFit();
        } else return false;

        // --- 2. Prévisions (Forecast) ---
        snprintf(url, sizeof(url), "%s/data/2.5/forecast?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", 
                 weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        http_request(url, resCodeForecast, resStringForecast);
        
        if (resCodeForecast > 0) {
            docForecast.clear();
            JsonDocument filter;
            filter["list"][0] = true; // On garde les éléments de la liste
            deserializeJson(docForecast, resStringForecast, DeserializationOption::Filter(filter));
            docForecast.shrinkToFit();
        } else return false;

        return true;
    }

    // Aide interne pour remplacer TimeLib par time.h
    const char* formatUnixTime(time_t t, const char* format) {
        static char buf[30];
        struct tm *timeinfo = localtime(&t);
        strftime(buf, sizeof(buf), format, timeinfo);
        return buf;
    }

    const char* get_sunrise() {
        time_t t = docCurrent["sys"]["sunrise"];
        return formatUnixTime(t, "%H:%M");
    }

    const char* get_sunset() {
        time_t t = docCurrent["sys"]["sunset"];
        return formatUnixTime(t, "%H:%M");
    }

    const unsigned char* get_situationIcon() {
        const char* icon = docCurrent["weather"][0]["icon"] | "00";
        if (strncmp(icon, "01", 2) == 0) return WeatherIcons_sun;
        if (strncmp(icon, "02", 2) == 0) return WeatherIcons_suncloud;
        if (strncmp(icon, "03", 2) == 0 || strncmp(icon, "04", 2) == 0) return WeatherIcons_cloud;
        if (strncmp(icon, "09", 2) == 0) return WeatherIcons_huge_rain;
        if (strncmp(icon, "10", 2) == 0) return WeatherIcons_rain;
        if (strncmp(icon, "11", 2) == 0) return WeatherIcons_thunder;
        if (strncmp(icon, "13", 2) == 0) return WeatherIcons_snow;
        if (strncmp(icon, "50", 2) == 0) return WeatherIcons_fog;
        return WeatherIcons_nothing;
    }

    const char* get_situationFont(int item = 0) {
        const char* icon = docForecast["list"][item]["weather"][0]["icon"] | "00";
        if (strncmp(icon, "01", 2) == 0) return "J";
        if (strncmp(icon, "02", 2) == 0) return "F";
        if (strncmp(icon, "03", 2) == 0 || strncmp(icon, "04", 2) == 0) return "A";
        if (strncmp(icon, "09", 2) == 0) return "B";
        if (strncmp(icon, "10", 2) == 0) return "G";
        if (strncmp(icon, "11", 2) == 0) return "I";
        if (strncmp(icon, "13", 2) == 0) return "H";
        if (strncmp(icon, "50", 2) == 0) return "C";
        return "Z";
    }

    const char* get_temperature(const char* element = "current", int item = 0) {
        static char buffer[15];
        
        // On force la conversion en float de chaque côté pour que les types correspondent
        float temp = (strcmp(element, "forecast") == 0) ? 
                      docForecast["list"][item]["main"]["temp"].as<float>() : 
                      docCurrent["main"]["temp"].as<float>();
                      
        snprintf(buffer, sizeof(buffer), "%.1f°C", temp);
        return buffer;
    }

    const char* get_temperatureRessenti(const char* element = "current", int item = 0) {
      static char buffer[10];

      float temperatureRessenti = (strcmp(element, "forecast") == 0) ? 
              docForecast["list"][item]["main"]["feels_like"].as<float>() : 
              docCurrent["main"]["feels_like"].as<float>();

      float roundedTemperatureRessenti = round(temperatureRessenti * 10) / 10.0;
      snprintf(buffer, sizeof(buffer), "%.1f°C", roundedTemperatureRessenti);
      return buffer;
    }

    const char* get_humidity(const char* element = "current", int item = 0) {
        static char buffer[10];
        
        // Idem pour l'humidité (en int ou float)
        int hum = (strcmp(element, "forecast") == 0) ? 
                  docForecast["list"][item]["main"]["humidity"].as<int>() : 
                  docCurrent["main"]["humidity"].as<int>();
                  
        snprintf(buffer, sizeof(buffer), "%d %%", hum);
        return buffer;
    }

    const char* get_windSpeed(const char* element = "current", int item = 0) {
        static char buffer[15];
        
        float speed = (strcmp(element, "forecast") == 0) ? 
                      docForecast["list"][item]["wind"]["speed"].as<float>() : 
                      docCurrent["wind"]["speed"].as<float>();
                      
        snprintf(buffer, sizeof(buffer), "%.0f km/h", speed * 3.6);
        return buffer;
    }

    const char* get_temperatureAll(const char* element = "current", int item = 0) {
      static char buffer_temperature[50];
      snprintf(buffer_temperature, sizeof(buffer_temperature), "%s (%s)", get_temperature(element, item), get_temperatureRessenti(element, item));
      return buffer_temperature;
    }

    const char* get_pressure(const char* element = "current", int item = 0) {
      static char buffer[10];

      float pressure = (strcmp(element, "forecast") == 0) ? 
              docForecast["list"][item]["main"]["pressure"].as<float>() : 
              docCurrent["main"]["pressure"].as<float>();

      float roundedPressure = round(pressure * 10) / 10.0;

      snprintf(buffer, sizeof(buffer), "%.0f HPa", roundedPressure);
      return buffer;
    }

    int get_windDirection(const char* element = "current", int item = 0) {
      int windDirection = (strcmp(element, "forecast") == 0) ? 
        docForecast["list"][item]["wind"]["deg"].as<float>() : 
        docCurrent["wind"]["deg"].as<float>();

      return windDirection;
    }

    int get_nbElementForecast() {
        // On récupère la taille du tableau "list" contenu dans le document
        size_t nbElements = docForecast["list"].size(); 

        // On applique votre limite à 9
        if (nbElements > 9) {
            return 9;
        } 
        return (int)nbElements;
    }

    const char* get_dateForecast(int item = 0) {
        time_t t = docForecast["list"][item]["dt"];
        return formatUnixTime(t, "%d/%m %H:%M");
    }

    int get_dtForecast(int item = 0) {
        return docForecast["list"][item]["dt"];
    }
};