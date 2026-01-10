class Birthday {
public:
    JsonDocument docBirthday;

    bool refresh() {
        int responseBirthdayCode;
        String responseBirthdayString;
        
        http_request(birthday_url, responseBirthdayCode, responseBirthdayString);
        
        if (responseBirthdayCode > 0) {
            docBirthday.clear(); // On vide les anniversaires précédents
            
            int pos = 0;
            while ((pos = responseBirthdayString.indexOf("BEGIN:VEVENT", pos)) != -1) {
                int eventEnd = responseBirthdayString.indexOf("END:VEVENT", pos);
                if (eventEnd == -1) break;

                String eventBlock = responseBirthdayString.substring(pos, eventEnd);

                // Extraction des champs (format ICS standard)
                String nom = extractField(eventBlock, "SUMMARY:", "\n");
                String date_event = extractField(eventBlock, "DTSTART;VALUE=DATE:", "\n");
                
                if (nom.length() > 0 && date_event.length() > 0) {
                    // On garde MMDD (ex: 0110 pour le 10 janvier)
                    String date_event_court = date_event.substring(4, 8);
                    
                    // Si la date n'existe pas encore, on l'initialise en tant que tableau
                    if (docBirthday[date_event_court].isNull()) {
                        docBirthday[date_event_court].to<JsonArray>();
                    }

                    // On ajoute le nom au tableau
                    docBirthday[date_event_court].add(nom);
                }
                pos = eventEnd;
            }

            docBirthday.shrinkToFit();
            return true;
        } 
        return false;
    }

    const char* get_BirthdayOfTheDay(const char* date_event) {
        // date_event doit être au format "MMDD"
        if (docBirthday[date_event].isNull()) {
          return "-"; 
        }

        JsonArray namesArray = docBirthday[date_event].as<JsonArray>();

        static char result[256];
        int pos = 0;
        result[0] = '\0';

        pos += snprintf(result + pos, sizeof(result) - pos, "Anniversaire : ");

        // Parcours du tableau des prénoms
        bool first = true;
        for (const char* name : namesArray) {
            if (!first) {
                pos += snprintf(result + pos, sizeof(result) - pos, ", ");
            }
            pos += snprintf(result + pos, sizeof(result) - pos, "%s", name);
            first = false;
        }

        Serial.printf("> Affichage anniversaire -> %s\n", result);
        return result;
    }

private:
    String extractField(const String& data, const String& startMarker, const String& endMarker) {
        int startIdx = data.indexOf(startMarker);
        if (startIdx == -1) return "";

        startIdx += startMarker.length();
        int endIdx = data.indexOf(endMarker, startIdx);
        if (endIdx == -1) return "";

        String result = data.substring(startIdx, endIdx);
        // Nettoyage crucial pour les fichiers ICS (supprime les \r et espaces)
        result.replace("\r", "");
        result.trim();
        return result;
    }
};