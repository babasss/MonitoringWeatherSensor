class Nominis {
public:
    JsonDocument docNominis;

    bool refresh() {
        int responseCode;
        String responseString;
        
        http_request(nominis_url, responseCode, responseString);
        
        if (responseCode > 0) {
            // Optionnel : Vous pouvez vider le document avant de le réutiliser
            docNominis.clear();

            // En V7, le filtrage a aussi légèrement changé de syntaxe
            // mais deserializeJson reste très similaire.
            DeserializationError error = deserializeJson(docNominis, responseString);
            
            if (error) {
                Serial.print("Erreur de parsing Nominis: ");
                Serial.println(error.f_str());
                return false;
            }

            docNominis.shrinkToFit(); // Toujours utile pour optimiser la RAM
            return true;
        } 
        return false;
    }

    const char* get_saintDuJour() {
        // Accès direct beaucoup plus simple
        const char* nom = docNominis["response"]["saintdujour"]["nom"];
        return nom ? nom : "Inconnu"; // Sécurité si le champ est absent
    }
};