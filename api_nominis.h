class Nominis {
public:
    String saintdujour;
    DynamicJsonDocument* jsonResponse;

    Nominis() {
        jsonResponse = new DynamicJsonDocument(512); // Initialiser jsonResponse dans le constructeur
    }

    void refresh() {
        int responseCode;
        String responseString;
        
        http_request(nominis_url, responseCode, responseString);
        
        if (responseCode > 0) {
            StaticJsonDocument<200> filter;
            filter["response"]["saintdujour"]["nom"] = true;
            deserializeJson(*jsonResponse, responseString); // Dereference le pointeur ici
            (*jsonResponse).shrinkToFit();
        }
    }

    const char* get_saintDuJour() {
        const char* buffer = (*jsonResponse)["response"]["saintdujour"]["nom"];
        return buffer;
    }
};