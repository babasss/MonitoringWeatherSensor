class Domoticz {
public:
    // En V7, un seul type de document pour tout
    JsonDocument jsonSensor; 
    String dateHeureServer;

    Domoticz() {
        // Plus besoin d'initialisation complexe dans le constructeur
    }

    bool refresh() {
        int responseCode;
        String responseString;
        char header[100];
        
        // Document temporaire pour la réponse HTTP
        JsonDocument jsonResponse;

        snprintf(header, sizeof(header), "Basic %s", domoticz_apikey);
        String domoticzHeader[] = {"Authorization", header};

        http_request(domoticz_url, responseCode, responseString, domoticzHeader);

        if (responseCode > 0) {
            // --- NOUVELLE SYNTAXE DE FILTRAGE V7 ---
            JsonDocument filter;
            filter["ServerTime"] = true;
            filter["result"][0]["Name"] = true;
            filter["result"][0]["Temp"] = true;
            filter["result"][0]["Humidity"] = true;
            filter["result"][0]["LastUpdate"] = true;

            DeserializationError error = deserializeJson(jsonResponse, responseString, DeserializationOption::Filter(filter));

            if (error) {
                Serial.print(F("Domoticz Parsing failed: "));
                Serial.println(error.f_str());
                return false;
            }

            // On stocke ServerTime en String pour qu'il survive après la fonction
            dateHeureServer = jsonResponse["ServerTime"].as<String>();

            // On vide l'ancien stockage des capteurs
            jsonSensor.clear();

            JsonArray resultArray = jsonResponse["result"];
            for (JsonObject item : resultArray) {
                String pieceName = item["Name"];
                
                // --- NOUVELLE SYNTAXE V7 POUR CRÉER UN OBJET IMBRIQUÉ ---
                JsonObject pieceObj = jsonSensor[pieceName].to<JsonObject>();

                pieceObj["Temp"] = item["Temp"];
                pieceObj["Humidity"] = item["Humidity"];
                pieceObj["LastUpdate"] = item["LastUpdate"];
            }
            
            jsonSensor.shrinkToFit();
            return true;
        } 
        return false;
    }

    const char* get_Sensor(const char* piece = "", const char* type = "Temp", bool breakChar = false) {
        // Vérification si la pièce existe dans notre stockage
        if (piece[0] == '\0' || jsonSensor[piece].isNull()) {
            return "";
        }

        // Conversion des dates (assurez-vous que convertToEpoch accepte const char*)
        time_t sensorEpoch = convertToEpoch(jsonSensor[piece]["LastUpdate"]);
        time_t nowEpoch = convertToEpoch(dateHeureServer.c_str());

        double diffHours = difftime(nowEpoch, sensorEpoch) / 3600.0;

        // Si la donnée a plus de 12h, on considère qu'elle est invalide
        if (diffHours > 12) {
            return "";
        }
        
        static char buffer[30];
        const char* separateur = breakChar ? " | " : "";
        float element = jsonSensor[piece][type];
        
        // Arrondi à 1 décimale
        float roundedElement = round(element * 10.0f) / 10.0f;
        
        if (strcmp(type, "Temp") == 0) {
            snprintf(buffer, sizeof(buffer), "%.1f°C%s", roundedElement, separateur);
        } else {
            snprintf(buffer, sizeof(buffer), "%.0f%%%s", roundedElement, separateur); 
        }

        return buffer;
    }
};