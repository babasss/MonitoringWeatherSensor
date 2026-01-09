class DateWeb {
  public:
    String date_jour;
    String heure;
    String date_jour_heure;
    int offset;

    bool refresh() {
  
      bool success;
      int responseCode;
      String responseString;
      //String date_jour_string;
      http_request(date_url, responseCode, responseString);

      if (responseCode > 0) {
        DynamicJsonDocument jsonResponse(2048);
        DeserializationError error = deserializeJson(jsonResponse, responseString);

        if (!error) {
          date_jour = jsonResponse["datetime"].as<String>().substring(0, 10); // Extraire la date au format YYYY-MM-DD // currentLocalTime pour timeapi.io
          heure = jsonResponse["datetime"].as<String>().substring(11, 16); 
          date_jour_heure = jsonResponse["datetime"].as<String>().substring(0, 19);
          date_jour_heure.replace("T"," ");
          //offset = jsonResponse["datetime"].as<String>().substring(27, 29).toInt()*3600; 
          offset =  jsonResponse["raw_offset"].as<String>().toInt();

          //Serial.printf("date_jour : %s - heure : %s - date_jour_heure : %s \n", date_jour.c_str(), heure.c_str(), date_jour_heure.c_str());

          success = (date_jour.length() > 0);
        } else {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          success = false;
        }
      } else {
          Serial.print(F("Error on HTTP request: "));
          Serial.println(responseCode);
          success = false;
      }

      return success;
    }

    const char* get_dateJourHeure() {
        static char buffer[50];
        //date_jour_heure.replace("T", " ");
        snprintf(buffer, sizeof(buffer), "%s %s:00", date_jour.c_str(), heure.c_str());
        //Serial.printf("get_dateJourHeure : %s \n", buffer);
        return buffer;
    }

    /*const char* get_dateJour() {
        static char buffer[20];
        snprintf(buffer, sizeof(buffer), "%s", date_jour.c_str());
        Serial.printf("date_jour : %s \n", buffer);
        return buffer;
    }*/

    const char* get_dateHeure() {
        static char buffer[50];
        int month = date_jour.substring(5, 7).toInt();
        int day = date_jour.substring(8, 10).toInt();

        snprintf(buffer, sizeof(buffer), "Dernière mise à jour : %02d/%02d %s", day, month, heure.c_str());
        //Serial.printf("get_dateHeure : %s \n", buffer);
        return buffer;
    }

    const char* get_dateCalendar() {
        static char buffer[50];
        //int year = date_jour.substring(1, 4).toInt();
        int month = date_jour.substring(5, 7).toInt();
        int day = date_jour.substring(8, 10).toInt();

        snprintf(buffer, sizeof(buffer), "%02d%02d", month, day);
        //Serial.printf("dateCalendar : %s \n", buffer);
        return buffer;
    }

    const char* get_dateJourLettre() {
        static char buffer[50];
        int year = date_jour.substring(0, 4).toInt();
        int month = date_jour.substring(5, 7).toInt();
        int day = date_jour.substring(8, 10).toInt();
        //int year = 2024; int month = 12; int day = 22;
        String daysOfWeek[] = {"samedi", "dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi"};
        String monthsOfYear[] = {"janvier", "février", "mars", "avril", "mai", "juin", "juillet", "août", "septembre", "octobre", "novembre", "décembre"};

        int dayOfWeek = calculateDayOfWeek(year, month, day);

        //const char* dateLettre = daysOfWeek[dayOfWeek] + " " + day + " " + monthsOfYear[month - 1];

        snprintf(buffer, sizeof(buffer), "%s %d %s", daysOfWeek[dayOfWeek], day, monthsOfYear[month - 1]);
        return buffer;
    }

private:
    int calculateDayOfWeek(int year, int month, int day) const {
        if (month < 3) {
            month += 12;
            year -= 1;
        }
        int k = year % 100;
        int j = year / 100;
        int f = day + (13 * (month + 1)) / 5 + k + (k / 4) + (j / 4) + (5 * j);
        return f % 7;
    }
};