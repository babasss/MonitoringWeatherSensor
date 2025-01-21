class Nominis {
public:
    //String saintdujour;
    DynamicJsonDocument* jsonNominis;

    Nominis() {
        jsonNominis = new DynamicJsonDocument(512); // Initialiser jsonResponse dans le constructeur
    }

    bool refresh() {
        int responseCode;
        String responseString;
        
        http_request(nominis_url, responseCode, responseString);
        
        if (responseCode > 0) {
            StaticJsonDocument<200> filter;
            filter["response"]["saintdujour"]["nom"] = true;
            deserializeJson(*jsonNominis, responseString); // Dereference le pointeur ici
            (*jsonNominis).shrinkToFit();
            return true;
        } else {
          return false;
        }
        
    }

    const char* get_saintDuJour() {
        const char* buffer = (*jsonNominis)["response"]["saintdujour"]["nom"];
        return buffer;
    }
};