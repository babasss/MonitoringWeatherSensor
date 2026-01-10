class AirPollution {
public:
    // Documents persistants pour la classe
    JsonDocument docAirCurrent;
    JsonDocument docAirForecast;

    AirPollution() {}

    bool refresh() {
        int responseCode;
        String responseString;
        char url[300];

        // --- 1. RÉCUPÉRATION AIR POLLUTION ACTUELLE ---
        snprintf(url, sizeof(url), "%s/data/2.5/air_pollution?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", 
                 weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        
        http_request(url, responseCode, responseString);
        
        if (responseCode > 0) {
            docAirCurrent.clear();
            deserializeJson(docAirCurrent, responseString);
            docAirCurrent.shrinkToFit();
        } else {
            return false;
        }

        // --- 2. RÉCUPÉRATION PRÉVISIONS (FORECAST) ---
        snprintf(url, sizeof(url), "%s/data/2.5/air_pollution/forecast?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", 
                 weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        
        http_request(url, responseCode, responseString);
        
        if (responseCode > 0) {
            JsonDocument jsonResponse; // Document temporaire pour le parsing du flux complet
            JsonDocument filter;       // Nouveau système de filtre V7
            
            filter["list"][0]["dt"] = true; 
            filter["list"][0]["main"]["aqi"] = true;

            DeserializationError error = deserializeJson(jsonResponse, responseString, DeserializationOption::Filter(filter));
            
            if (!error) {
                docAirForecast.clear(); // On vide les prévisions calculées précédentes
                
                JsonArray resultArray = jsonResponse["list"];
                for (JsonObject item : resultArray) {
                    // Utilisation du timestamp (dt) comme clé de l'objet
                    String dtAnalyse = item["dt"].as<String>();
                    
                    // Syntaxe V7 pour créer l'entrée
                    docAirForecast[dtAnalyse]["aqi"] = item["main"]["aqi"];
                }
                docAirForecast.shrinkToFit();
            } else {
                return false;
            }
        } else {
            return false;
        }

        return true;
    }

    int get_airPollution(const char* element = "current", int dt = 0) {
        if (strcmp(element, "forecast") == 0) {
            String dtKey = String(dt);
            // On vérifie si la prévision existe pour ce timestamp
            return docAirForecast[dtKey]["aqi"] | 0;
        } else {
            return docAirCurrent["list"][0]["main"]["aqi"] | 0; // | 0 est une valeur par défaut si absent
        }
    }
};