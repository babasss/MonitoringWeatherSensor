class Birthday {
public:
    //String saintdujour;
    DynamicJsonDocument* jsonBirthday;

    Birthday() {
        jsonBirthday = new DynamicJsonDocument(512); // Initialiser jsonResponse dans le constructeur
    }

    bool refresh() {
        int responseBirthdayCode;
        String responseBirthdayString;
        
        http_request(birthday_url, responseBirthdayCode, responseBirthdayString);
        
        if (responseBirthdayCode > 0) {
          // Parse les champs `SUMMARY` et `DTSTART`
          int pos = 0;
          while ((pos = responseBirthdayString.indexOf("BEGIN:VEVENT", pos)) != -1) {
            // Trouver la fin de l'événement
            int eventEnd = responseBirthdayString.indexOf("END:VEVENT", pos);
            if (eventEnd == -1) break;

            // Extraire le bloc de l'événement
            String eventBlock = responseBirthdayString.substring(pos, eventEnd);

            // Extraire les champs SUMMARY et DTSTART
            String nom = extractField(eventBlock, "SUMMARY:", "\n");
            String date_event = extractField(eventBlock, "DTSTART;VALUE=DATE:", "\n");
            
            //Serial.print(date_event);Serial.print(" -- ");Serial.print(nom);Serial.println("_\n");
            // Vérifier si les champs ont été trouvés
            if (nom.length() > 0 && date_event.length() > 0) {
              String date_event_court = date_event.substring(4, 8);
              // Vérifier si la date existe déjà dans le JSON
              if (!(*jsonBirthday)[date_event_court]) {
                // Créer un tableau pour cette date si elle n'existe pas
                (*jsonBirthday)[date_event_court] = JsonArray();
              }

              // Ajouter le nom au tableau correspondant à la date
              (*jsonBirthday)[date_event_court].add(nom);
            }

            // Mettre à jour la position
            pos = eventEnd;
          }

          (*jsonBirthday).shrinkToFit();

          //serializeJson((*jsonBirthday), Serial);
          return true;
        } else {
          return false;
        }
        
    }

    const char* get_BirthdayOfTheDay(const char* date_event) {
    // Vérifier si la clé existe dans l'objet JSON
    if (!(*jsonBirthday).containsKey(date_event)) {
      return "-"; // Retourner "-" si la date n'existe pas
    }

    // Récupérer le tableau d'événements pour cette date
    JsonArray namesArray = (*jsonBirthday)[date_event].as<JsonArray>();

    // Buffer statique (attention : écrasé à chaque appel)
    static char result[256];
    size_t pos = 0;
    result[0] = '\0';

    // Ajouter le préfixe
    pos += snprintf(result + pos, sizeof(result) - pos, "Anniversaire : ");

    // Ajouter les prénoms
    for (size_t i = 0; i < namesArray.size(); i++) {
      const char* name = namesArray[i];
      pos += snprintf(result + pos, sizeof(result) - pos, "%s", name);
      if (i < namesArray.size() - 1) {
        pos += snprintf(result + pos, sizeof(result) - pos, ", ");
      }
    }

    // Debug
    Serial.printf("> Affichage anniversaire -> %s\n", result);

    return result;
  }

private:
  String icsData;

  // Fonction pour extraire une valeur spécifique entre deux marqueurs
  String extractField(const String& data, const String& startMarker, const String& endMarker) {
    int startIdx = data.indexOf(startMarker);
    if (startIdx == -1) return ""; // Marqueur de début non trouvé

    startIdx += startMarker.length();
    int endIdx = data.indexOf(endMarker, startIdx);
    if (endIdx == -1) return ""; // Marqueur de fin non trouvé

    String result = data.substring(startIdx, endIdx);
    result.trim(); // Supprime les espaces blancs directement sur la chaîne
    return result;
  }
};