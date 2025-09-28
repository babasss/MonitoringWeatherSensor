class Domoticz {
private:
  DateWeb dateweb;

public:
    DynamicJsonDocument* jsonSensor;
    //MyClass(DateWeb* dw) : dateweb(dw) {} // constructeur
    //DynamicJsonDocument* jsonResponse;

    Domoticz() {
        jsonSensor = new DynamicJsonDocument(512); // Initialiser jsonSensor dans le constructeur
        //jsonResponse = new DynamicJsonDocument(2048); // Initialiser jsonResponse dans le constructeur
    }

    bool refresh() {
        int responseCode;
        String responseString;
        char header[100];
        DynamicJsonDocument jsonResponse(2048);

        snprintf(header, sizeof(header), "Basic %s", domoticz_apikey);
        String domoticzHeader[] = {"Authorization", header};

        http_request(domoticz_url, responseCode, responseString, domoticzHeader);
        //Serial.println(responseString);

        if (responseCode > 0) {
            StaticJsonDocument<200> filter;
            filter["result"][0]["Name"] = true;
            filter["result"][0]["Temp"] = true;
            filter["result"][0]["Humidity"] = true;
            filter["result"][0]["LastUpdate"] = true;
            deserializeJson(jsonResponse, responseString, DeserializationOption::Filter(filter)); // Dereference le pointeur ici

            JsonArray resultArray = jsonResponse["result"];
            for (size_t t = 0; t < resultArray.size(); t++) {
              // Récupérez les informations de chaque pièce
              String pieceName = resultArray[t]["Name"];
              //float temp = resultArray[t]["Temp"];
              //float humidity = resultArray[t]["Humidity"];

              // Créez un objet imbriqué pour chaque pièce dans jsonSensor
              JsonObject piece = (*jsonSensor).createNestedObject(pieceName);

              piece["Temp"] = resultArray[t]["Temp"];
              piece["Humidity"] = resultArray[t]["Humidity"];
              piece["LastUpdate"] = resultArray[t]["LastUpdate"];
            }
            
            //serializeJson((*jsonSensor), Serial);
            return true;
        } else {
          return false;
        }
    }

    const char* get_Sensor(const char* piece = "", const char* type = "Temp", bool breakChar = false, const char* dateJourHeure = "") {
      // On teste si la donnée n'est pas trop ancienne
      time_t sensorEpoch = convertToEpoch((*jsonSensor)[piece]["LastUpdate"]);
      time_t nowEpoch = convertToEpoch(dateJourHeure);

      double diffHours = difftime(nowEpoch, sensorEpoch) / 3600.0;

      if ( piece == "" || diffHours > 12 ) {
        return "";
      }
      else {
        static char buffer[20];
        char* separateur = "";
        float element =  (*jsonSensor)[piece][type];
        float roundedElement = round(element * 10) / 10.0;
        
        if (breakChar == true) { separateur = " | "; }
        
        if ( type == "Temp" ) {
          snprintf(buffer, sizeof(buffer), "%.1f°C %s", roundedElement, separateur);
        } else {
          snprintf(buffer, sizeof(buffer), "%.0f %s",  roundedElement, "%"); 
        }

        return buffer;
      }
    }
};