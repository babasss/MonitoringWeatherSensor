class AirPollution {
public:
    DynamicJsonDocument* jsonResponseAirPollutionCurrent;
    DynamicJsonDocument* jsonCalculatedAirPollutionForecast;

    AirPollution() {
        jsonResponseAirPollutionCurrent = new DynamicJsonDocument(512); // Initialiser jsonResponse dans le constructeur
        jsonCalculatedAirPollutionForecast = new DynamicJsonDocument(1024); // Initialiser jsonResponse dans le constructeur
    }

    void refresh() {
        int responseAirPollutionCodeCurrent;
        String responseAirPollutionStringCurrent;
        char url_airPollution[256];

        // Construire l'URL de la requête
        snprintf(url_airPollution, sizeof(url_airPollution), "%s/data/2.5/air_pollution?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        http_request(url_airPollution, responseAirPollutionCodeCurrent, responseAirPollutionStringCurrent);
        if (responseAirPollutionCodeCurrent > 0) {
            //Serial.println(responseStringCurrent);
            deserializeJson(*jsonResponseAirPollutionCurrent, responseAirPollutionStringCurrent); // Dereference le pointeur ici
            (*jsonResponseAirPollutionCurrent).shrinkToFit();
        }
        
        
        // Construire l'URL de la requête
        
        int responseAirPollutionCodeForecast;
        String responseAirPollutionStringForecast;
        DynamicJsonDocument jsonResponseAirPollutionForecast(1024);
        snprintf(url_airPollution, sizeof(url_airPollution), "%s/data/2.5/air_pollution/forecast?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        http_request(url_airPollution, responseAirPollutionCodeForecast, responseAirPollutionStringForecast);
        if (responseAirPollutionCodeForecast > 0) {
            //Serial.println(responseStringForecast);
            StaticJsonDocument<200> filter;
            filter["list"][0]["dt"] = true; 
            filter["list"][0]["main"]["aqi"] = true;
            deserializeJson(jsonResponseAirPollutionForecast, responseAirPollutionStringForecast, DeserializationOption::Filter(filter)); // Dereference le pointeur ici
            // Boucle pour passer les données AirPollution en tableau associatif avec la date
            JsonArray resultArray = jsonResponseAirPollutionForecast["list"];
            for (size_t t = 0; t < resultArray.size(); t++) {
              // Récupérez les informations de chaque analyse
              String dtAnalyse = resultArray[t]["dt"]; //Utilisation d'une string car je ne sais pas faire autrement
              // Créez un objet imbriqué pour chaque pièce dans jsonCalculatedAirPollutionForecast
              JsonObject analyse = (*jsonCalculatedAirPollutionForecast).createNestedObject(dtAnalyse);
              analyse["aqi"] = resultArray[t]["main"]["aqi"];
            }
            //serializeJson(jsonResponseAirPollutionForecast, Serial);
            //serializeJson(*jsonCalculatedAirPollutionForecast, Serial);
        }
    }

    int get_airPollution(const char* element = "current", int dt = 0) {
      int buffer;
      if ( element == "forecast" ) {
        String dtAnalyse = String(dt);
        buffer = (*jsonCalculatedAirPollutionForecast)[dtAnalyse]["aqi"];
      }
      else 
      {
        buffer = (*jsonResponseAirPollutionCurrent)["list"][0]["main"]["aqi"];
      }
      //Serial.println(buffer);
      return buffer;
    }
};