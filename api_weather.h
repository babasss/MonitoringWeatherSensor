#include "WeatherIcons.h"  // WeatherIcons
#include <TimeLib.h>

class Weather {
public:
    DynamicJsonDocument* jsonResponseCurrent;
    DynamicJsonDocument* jsonResponseForecast;

    Weather() {
        jsonResponseCurrent = new DynamicJsonDocument(512); // Initialiser jsonResponse dans le constructeur
        jsonResponseForecast = new DynamicJsonDocument(2048); // Initialiser jsonResponse dans le constructeur
    }

    void refresh() {
        int responseCodeCurrent;
        int responseCodeForecast;
        String responseStringCurrent;
        String responseStringForecast;
        char url[256];

        // Construire l'URL de la requête
        snprintf(url, sizeof(url), "%s/data/2.5/weather?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        http_request(url, responseCodeCurrent, responseStringCurrent);
        if (responseCodeCurrent > 0) {
            deserializeJson(*jsonResponseCurrent, responseStringCurrent); // Dereference le pointeur ici
            (*jsonResponseCurrent).shrinkToFit();
        }

        // Construire l'URL de la requête
        snprintf(url, sizeof(url), "%s/data/2.5/forecast?appid=%s&lat=%s&lon=%s&lang=%s&units=%s&mode=json", weather_url, weather_apikey, weather_lat, weather_long, weather_lang, weather_unit);
        http_request(url, responseCodeForecast, responseStringForecast);
        if (responseCodeForecast > 0) {
            //Serial.println(responseStringForecast);
            StaticJsonDocument<200> filter;
            filter["list"][0] = true;
            deserializeJson(*jsonResponseForecast, responseStringForecast, DeserializationOption::Filter(filter)); // Dereference le pointeur ici
            (*jsonResponseForecast).shrinkToFit();
            //jsonResponseForecast = (*jsonResponseForecast)["list"];
        }
    }

    const char* get_sunrise(int offset = 0) {
        static char buffer[10];
        //return (*jsonResponseCurrent)["sys"]["sunrise"].as<String>();
        int dtSunrise = (*jsonResponseCurrent)["sys"]["sunrise"];
        time_t dateSunrise = dtSunrise + offset;
        //Serial.print(dtSunrise);Serial.print("--");Serial.print(dateSunrise);Serial.print("--");Serial.println(offset);
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hour(dateSunrise), minute(dateSunrise));
        return buffer;
    }

    const char* get_sunset(int offset = 0) {
        static char buffer[10];
        //return (*jsonResponseCurrent)["sys"]["sunset"].as<String>();
        int dtSunset = (*jsonResponseCurrent)["sys"]["sunset"];
        time_t dateSunset = dtSunset + offset;
        snprintf(buffer, sizeof(buffer), "%02d:%02d", hour(dateSunset), minute(dateSunset));
        return buffer;
    }

    const unsigned char* get_situationIcon() {
        const unsigned char* WeatherIcons_return;
        
        String WeatherIconPrefix = (*jsonResponseCurrent)["weather"][0]["icon"].as<String>().substring(0, 2);
        //const char* WeatherIconPrefix = iconJSON;

        //char WeatherIconPrefix[3]; // Taille 3 pour 2 caractères + '\0'
        //strncpy(WeatherIconPrefix, (*jsonResponseCurrent)["weather"][0]["icon"], 2); // Copie les deux premiers caractères
        //WeatherIconPrefix[2] = '\0'; // Ajouter le caractère nul de fin

        //const char WeatherIconPrefix[] = "03";
        //Serial.println(WeatherIconPrefix);

        if ( WeatherIconPrefix == "01" ) { //sun
            WeatherIcons_return = WeatherIcons_sun;
            //Serial.println("Sun");
        } else if (  WeatherIconPrefix == "02" ) { //sun & cloud
            WeatherIcons_return = WeatherIcons_suncloud;
        } else if ( WeatherIconPrefix == "03" || WeatherIconPrefix == "04" ) { // clouds
            //Serial.println("Cloud");
            WeatherIcons_return = WeatherIcons_cloud;
        } else if (  WeatherIconPrefix == "09" ) { // huge rain
            WeatherIcons_return = WeatherIcons_huge_rain;
        } else if (  WeatherIconPrefix == "10" ) { // rain
            WeatherIcons_return = WeatherIcons_rain;
        } else if (  WeatherIconPrefix == "11" ) { // thunder
            WeatherIcons_return = WeatherIcons_thunder;
        } else if (  WeatherIconPrefix == "13" ) { // snow
            WeatherIcons_return = WeatherIcons_snow;
        } else if (  WeatherIconPrefix == "50" ) { // fog
            WeatherIcons_return = WeatherIcons_fog;
        } else { 
            //Serial.println("Nothing");
            WeatherIcons_return = WeatherIcons_nothing;
        }
        //WeatherIcons_return = WeatherIcons_sun;
        return WeatherIcons_return;
    }

    const char* get_situationFont(int item = 0) {
        const char* WeatherFont_return;
        
        String WeatherFontPrefix = (*jsonResponseForecast)["list"][item]["weather"][0]["icon"].as<String>().substring(0, 2);

        //char WeatherFontPrefix[3]; // Taille 3 pour 2 caractères + '\0'
        //strncpy(WeatherFontPrefix, (*jsonResponseForecast)["list"][item]["weather"]["icon"], 2); // Copie les deux premiers caractères
        //WeatherFontPrefix[2] = '\0'; // Ajouter le caractère nul de fin

        //String temp = (*jsonResponseForecast)["list"][item]["weather"][0]["icon"];
        //Serial.println(WeatherFontPrefix);

        if ( WeatherFontPrefix == "01" ) { //sun
            WeatherFont_return = "J";
        } else if (  WeatherFontPrefix == "02" ) { //sun & cloud
            WeatherFont_return = "F";
        } else if ( WeatherFontPrefix == "03" || WeatherFontPrefix == "04" ) { // clouds
            WeatherFont_return = "A";
        } else if (  WeatherFontPrefix == "09" ) { // huge rain
            WeatherFont_return = "B";
        } else if (  WeatherFontPrefix == "10" ) { // rain
            WeatherFont_return = "G";
        } else if (  WeatherFontPrefix == "11" ) { // thunder
            WeatherFont_return = "I";
        } else if (  WeatherFontPrefix == "13" ) { // snow
            WeatherFont_return = "H";
        } else if (  WeatherFontPrefix == "50" ) { // fog
            WeatherFont_return = "C";
        } else { 
            WeatherFont_return = "Z";
        }

        return WeatherFont_return;
    }

    const char* get_temperature(const char* element = "current", int item = 0) {
      static char buffer[10];
      float temperature;
      if ( element == "forecast" ) {
        temperature = (*jsonResponseForecast)["list"][item]["main"]["temp"];
      }
      else 
      {
        temperature = (*jsonResponseCurrent)["main"]["temp"];
      }
      float roundedTemperature = round(temperature * 10) / 10.0;
      snprintf(buffer, sizeof(buffer), "%.1f°C", roundedTemperature);
      return buffer;
    }

    
    const char* get_temperatureRessenti(const char* element = "current", int item = 0) {
      static char buffer[10];
      float temperatureRessenti;
      if ( element == "forecast" ) {
        temperatureRessenti = (*jsonResponseForecast)["list"][item]["main"]["feels_like"];
      }
      else 
      {
        temperatureRessenti = (*jsonResponseCurrent)["main"]["feels_like"];
      }
      float roundedTemperatureRessenti = round(temperatureRessenti * 10) / 10.0;
      snprintf(buffer, sizeof(buffer), "%.1f°C", roundedTemperatureRessenti);
      return buffer;
    }

    const char* get_temperatureAll(const char* element = "current", int item = 0) {
      static char buffer_temperature[50];
      snprintf(buffer_temperature, sizeof(buffer_temperature), "%s (%s)", get_temperature(element, item), get_temperatureRessenti(element, item));
      return buffer_temperature;
    }

    const char* get_humidity(const char* element = "current", int item = 0) {
      static char buffer[10];
      float humidity;
      if ( element == "forecast" ) {
        humidity = (*jsonResponseForecast)["list"][item]["main"]["humidity"];
      }
      else 
      {
        humidity = (*jsonResponseCurrent)["main"]["humidity"];
      }
      float roundedHumidity = round(humidity * 10) / 10.0;
      snprintf(buffer, sizeof(buffer), "%.0f %s", roundedHumidity, "%");
      return buffer;
    }

    const char* get_pressure(const char* element = "current", int item = 0) {
      static char buffer[10];
      float pressure;
      if ( element == "forecast" ) {
        pressure = (*jsonResponseForecast)["list"][item]["main"]["pressure"];
      }
      else 
      {
        pressure = (*jsonResponseCurrent)["main"]["pressure"];
      }
      float roundedPressure = round(pressure * 10) / 10.0;
      snprintf(buffer, sizeof(buffer), "%.0f HPa", roundedPressure);
      return buffer;
    }

    const char* get_windSpeed(const char* element = "current", int item = 0) {
      static char buffer[10];
      float windSpeed;
      if ( element == "forecast" ) {
        windSpeed = (*jsonResponseForecast)["list"][item]["wind"]["speed"];
      }
      else 
      {
        windSpeed = (*jsonResponseCurrent)["wind"]["speed"];
      }
      float roundedWindSpeed = round(windSpeed * 10 * 3.6) / 10.0;
      snprintf(buffer, sizeof(buffer), "%.0f km/h", roundedWindSpeed);
      return buffer;
    }

    int get_windDirection(const char* element = "current", int item = 0) {
      int windDirection;
      if ( element == "forecast" ) {
        windDirection = (*jsonResponseForecast)["list"][item]["wind"]["deg"];
      }
      else 
      {
        windDirection = (*jsonResponseCurrent)["wind"]["deg"];
      }
      return windDirection;
    }

    int get_nbElementForecast() {
        int nbElementForecast = sizeof((*jsonResponseForecast));
        if ( nbElementForecast > 9 ) { return 9; } else { return nbElementForecast; }
    }

    const char* get_dateForecast(int item = 0, int offset = 0) {
        static char buffer[30];
        int dt = (*jsonResponseForecast)["list"][item]["dt"];
        time_t dateForecast = dt + offset;
        //Serial.print((*jsonResponseForecast)["list"][item]["dt"].as<String>()+"--");Serial.print(dateForecast);Serial.print("--");Serial.println(offset);
        //serializeJson((*jsonResponseForecast)["list"][item]["dt_txt"], Serial);
        snprintf(buffer, sizeof(buffer), "%02d/%02d %02d:%02d", day(dateForecast), month(dateForecast), hour(dateForecast), minute(dateForecast));
        return buffer;
    }

    int get_dtForecast(int item = 0) {
        int dt = (*jsonResponseForecast)["list"][item]["dt"];
        return dt;
    }
};