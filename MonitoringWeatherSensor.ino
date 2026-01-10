/*
Reste à faire :
 - ligne noire sur les images
 - descendre un peu dessin
 - descendre un peu tableau
 - 
*/

#include <Arduino.h>
#include <ArduinoJson.h>
#include <time.h>
#include <WiFi.h>                     // Built-in
#include <WiFiClient.h>
//#include <NTPClient.h>
#include <esp_sntp.h>
#include <HTTPClient.h>

using namespace ArduinoJson;

/* Fonctions perso */
#include "config.h"  // Config
#include "wifi_connect.h"
#include "functions.h"
#include "battery.h"
#include "WeatherIcons.h"

/* API perso */
//#include "api_date.h"
#include "api_nominis.h"
#include "api_weather.h"
#include "api_domoticz.h"
#include "api_airPollution.h"
#include "api_birthday.h"

//// Init WAKEUP
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  time_to_sleep /* Time ESP32 will go to sleep (in seconds) */

/////// Initialisation epaper ////////
// Author: Jean-Marc Zingg
// Library: https://github.com/ZinggJM/GxEPD2
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
//#include <Fonts/FreeMonoBold18pt7b.h>
#include "Fonts/MonitoringWeatherIcon18pt7b.h"
#include "additions/U8G2_FONTS_GFX.h"

// Initialisation de l'écran
//GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=5*/ 0, /*DC=*/22, /*RST=*/21, /*BUSY=*/13));  // GDEW075T7 800x480, EK79655 (GD7965)
//GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=*/12, /*DC=*/22, /*RST=*/21, /*BUSY=*/-25));  // GDEW075T7 800x480, EK79655 (GD7965)
GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=*/25, /*DC=*/22, /*RST=*/21, /*BUSY=*/4));  // GDEW075T7 800x480, EK79655 (GD7965)
U8G2_FONTS_GFX u8g2Fonts(display);

// Définition des classes
Nominis nominis;
//DateSyncNTP dateSync;
Weather weather;
Domoticz domoticz;
AirPollution airPollution;
Birthday birthday;

// Déclaration des variables
int display_width;
int display_height;
int currentSpace;

int screenWidth = 480;
int screenHeight = 800;
int WeatherIconSize = 150;

int TableauHaut = 170;

int zoneDessin_haut = 490;
int zoneDessin_gauche = 25;
int zoneDessin_width = 430;
int zoneDessin_height = 240;
int zoneDessin_epaisseur = 2;

int statusCount = 0;
int real_time_to_sleep = TIME_TO_SLEEP;

#define MAX_LENGTH 500  // Taille maximale par chaîne
RTC_DATA_ATTR char rtc_date[MAX_LENGTH + 1] = "";
RTC_DATA_ATTR char rtc_saintDuJour[MAX_LENGTH + 1] = "";
RTC_DATA_ATTR char rtc_birthday[MAX_LENGTH + 1] = "";

void setup() {
  // put your setup code here, to run once:
  delay(2000);  //Take some time to open up the Serial Monitor
  Serial.begin(115200);

  Serial.println("#WAKE UP CAUSE !!!");
  //print_wakeup_reason();

  // Init WAKEUP
  Serial.println("#DEBUT INIT");  

  /// Init écran ///
  //display.init(115200, true, 2, false);  // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.init(0, false, 2, false);  // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse


  display.setRotation(1);
  display.setFullWindow();
  display.setTextColor(GxEPD_BLACK);

  display_width = display.width();
  display_height = display.height();
  currentSpace = display_width - WeatherIconSize;


  /// Init font ///
  // Fonts from https://github.com/olikraus/u8g2/wiki/fntgrpadobex11
  u8g2Fonts.setFontMode(1);          // use u8g2 transparent mode (this is default)
  u8g2Fonts.setFontDirection(0);     // left to right (this is default)
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);  // apply Adafruit GFX color

  Serial.println("#FIN INIT");

  boucle();

  if ( statusCount > 0 )
  {
    real_time_to_sleep = 300;
  }
  else 
  {
    real_time_to_sleep = TIME_TO_SLEEP;
  }

  //Serial.print("\r\n#DEEP SLEEP for : "); Serial.print(real_time_to_sleep/60); Serial.println(" minutes");
  Serial.printf("\r\n#DEEP SLEEP for : %d minutes\n", real_time_to_sleep/60);

  esp_deep_sleep(real_time_to_sleep * uS_TO_S_FACTOR);
}

void boucle() {
  // put your main code here, to run repeatedly:

  int currentLine = 0;
  struct tm timeinfo;
  bool statusWifi;
  bool statusWeather;
  bool statusDomoticz;
  bool statusAirPollution;
  bool statusNominis;
  bool statusBirthday;
  bool statusDateSync;

  Serial.println("#DEBUT BOUCLE");

  if ( StartWiFi() == WL_CONNECTED ) 
  {  
    Serial.println("> Wifi connected");
    statusWifi = true;

    // 2. Initialisation NTP (Config système ESP32)
    configTzTime(ntpTzInfo, ntpServer);

    // 3. Attente de synchronisation (Timeout 10s pour préserver la batterie)
    int attempts = 0;
    while (attempts < maxRetries) {
      attempts++;
      Serial.printf("> DateSync : Tentative %d/%d\n", attempts, maxRetries);
      if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
          statusDateSync = true;
          break; // Succès, on sort de la boucle !
      }
      else
      { 
        statusDateSync = false;
      }
      delay(1000);
    }

    // 4. Affichage du résultat
    if ( statusDateSync ) {
      time_t now;
      time(&now);
      localtime_r(&now, &timeinfo);
      
      // Utilisation d'un buffer pour l'affichage propre
      char buf[64];
      strftime(buf, sizeof(buf), "%A %d %B %Y %H:%M:%S", &timeinfo);
      Serial.printf("Date synchronisée : %s\n", buf);
    }

    statusWeather = tryRefresh("Weather", []() { return weather.refresh(); });
    statusDomoticz = tryRefresh("Domoticz", []() { return domoticz.refresh(); });
    statusAirPollution = tryRefresh("AirPollution", []() { return airPollution.refresh(); });
    statusNominis = tryRefresh("Nominis", []() { return nominis.refresh(); });
    statusBirthday = tryRefresh("Birthday", []() { return birthday.refresh(); });

    if (!strcmp(rtc_date, get_dateCalendar(&timeinfo)) == 0) 
    {
      if ( statusDateSync ) { snprintf(rtc_date, MAX_LENGTH, get_dateCalendar(&timeinfo)); }
      if ( statusNominis ) { snprintf(rtc_saintDuJour, MAX_LENGTH, nominis.get_saintDuJour()); }
      if ( statusBirthday ) { snprintf(rtc_birthday, MAX_LENGTH, birthday.get_BirthdayOfTheDay(get_dateCalendar(&timeinfo))); }
    }

    statusCount = statusDateSync?0:1 + statusWeather?0:1 + statusDomoticz?0:1 + statusNominis?0:1 + statusBirthday?0:1;

    //Serial.print("Status after all refresh : "); Serial.println(status);
    Serial.printf("Status after all refresh : %d\n", statusCount);

    StopWiFi();
  }
  else 
  {
    statusWifi = false;
  }

  if ( !statusWifi )
  {
    
    Serial.println("#Wifi not connected or connection is broken ! Retry or exit !");

    display.firstPage();
    do {
      u8g2Fonts.setFont(u8g2_font_helvB18_tf);
      u8g2Fonts.setCursor(40, 300);
      u8g2Fonts.print("No connection : retry in 5 minutes");

      u8g2Fonts.setFont(u8g2_font_helvB12_tf);
      u8g2Fonts.setCursor(40, 400);
      String status_wifi = "Wifi : " + String(statusWifi);
      u8g2Fonts.print(status_wifi); 

      u8g2Fonts.setCursor(40, 440);
      String status_total = "Total : " + String(statusCount);
      u8g2Fonts.print(status_total); 

    } while (display.nextPage());
  }
  else if (batteryPercentageLevel() < 5) {

    Serial.println("#Battery Low !");

    display.firstPage();
    do {
      u8g2Fonts.setFont(u8g2_font_helvB18_tf);
      u8g2Fonts.setCursor(40, 300);
      u8g2Fonts.print("Battery Low !");
      u8g2Fonts.setFont(u8g2_font_helvB12_tf);
      u8g2Fonts.setCursor(40, 400);
      u8g2Fonts.print("Please plug me :-)"); 
    } while (display.nextPage());
  }
  else
  {
    display.firstPage();
    do {
      // Calcul de tout ce qui va s'écrir en 18pt

      Serial.println("> Header");
      // Date en lettre
      u8g2Fonts.setFont(u8g2_font_helvB18_tf);

      // DateSync
      currentLine = currentLine + 25;
      if ( !statusDateSync ) 
      {  u8g2Fonts.setCursor(WeatherIconSize + (currentSpace) / 2, currentLine); u8g2Fonts.print("No data"); }
      else
      {
        int dJLw = u8g2Fonts.getUTF8Width(get_dateJourLettre(&timeinfo));
        u8g2Fonts.setCursor(WeatherIconSize + (currentSpace - dJLw) / 2, currentLine);
        u8g2Fonts.print(get_dateJourLettre(&timeinfo));
      }

      // Saint du jour
      currentLine = currentLine + 22; //lineHeight9
      if ( !statusNominis ) 
      {  u8g2Fonts.setCursor(WeatherIconSize + (currentSpace) / 2, currentLine); u8g2Fonts.print("No data"); }
      else
      {
        u8g2Fonts.setFont(u8g2_font_helvB12_tf);
        int sDJw = u8g2Fonts.getUTF8Width(rtc_saintDuJour);
        u8g2Fonts.setCursor(WeatherIconSize + (currentSpace - sDJw) / 2, currentLine);
        u8g2Fonts.print(rtc_saintDuJour);
      }
      
      // Anniversaire
      currentLine = currentLine + 22; //lineHeight9
      if ( !statusBirthday ) 
      {  u8g2Fonts.setCursor(WeatherIconSize + (currentSpace) / 2, currentLine); u8g2Fonts.print("No data"); }
      else
      {
        int sBIw = u8g2Fonts.getUTF8Width(rtc_birthday);
        u8g2Fonts.setCursor(WeatherIconSize + (currentSpace - sBIw) / 2, currentLine);
        u8g2Fonts.print(rtc_birthday);
      }
      
      // Weather
      if ( !statusWeather ) 
      {  u8g2Fonts.setCursor(WeatherIconSize + (currentSpace) / 2, currentLine + 100); u8g2Fonts.print("No data"); }
      else
      {
        currentLine = currentLine + 10;
        display.fillRect(WeatherIconSize + 250, currentLine, zoneDessin_epaisseur, 75, GxEPD_BLACK); // séparation température

        // Température
        u8g2Fonts.setFont(u8g2_font_helvB18_tf); 
        currentLine = currentLine + 25; //lineHeight9
        int tCw = u8g2Fonts.getUTF8Width(weather.get_temperatureAll());
        u8g2Fonts.setCursor(WeatherIconSize + 50, currentLine);
        u8g2Fonts.print(weather.get_temperatureAll());

        u8g2Fonts.setFont(u8g2_font_helvB12_tr);
        u8g2Fonts.setCursor(WeatherIconSize + 260, currentLine);
        u8g2Fonts.print(weather.get_windSpeed());
            
        //Autre information : humidité - pression
        currentLine = currentLine + 25; 
        u8g2Fonts.setCursor(WeatherIconSize + 80, currentLine);
        u8g2Fonts.print(weather.get_humidity());

        u8g2Fonts.setCursor(WeatherIconSize + 150, currentLine);
        u8g2Fonts.print(weather.get_pressure());

        //u8g2Fonts.setCursor(WeatherIconSize + 260, currentLine + 30);
        int x = WeatherIconSize + 290;
        int y = currentLine + 5;
        int r = 20;
        float angleRadians = (90 - weather.get_windDirection()) * PI / 180.0;
        //Serial.println(weather.get_windDirection());
        //Cercle rose des vents
        display.fillCircle(x, y, r+4, GxEPD_BLACK); r++;
        display.fillCircle(x, y, r, GxEPD_WHITE); r++;

        //Direction du vent
        display.drawLine(x + r*cos(angleRadians), y - r*sin(angleRadians), x + r*cos(angleRadians + PI), y - r*sin(angleRadians + PI), GxEPD_BLACK);
        if(cos(angleRadians) < 0.5) { 
          display.drawLine(x - 1 + r*cos(angleRadians), y - r*sin(angleRadians), x - 1 + r*cos(angleRadians + PI), y - r*sin(angleRadians + PI), GxEPD_BLACK);
          display.drawLine(x + 1 + r*cos(angleRadians), y - r*sin(angleRadians), x + 1 + r*cos(angleRadians + PI), y - r*sin(angleRadians + PI), GxEPD_BLACK);
        } else {
          display.drawLine(x + r*cos(angleRadians), y + 1 - r*sin(angleRadians), x + r*cos(angleRadians + PI), y + 1 - r*sin(angleRadians + PI), GxEPD_BLACK);
          display.drawLine(x + r*cos(angleRadians), y - 1 - r*sin(angleRadians), x + r*cos(angleRadians + PI), y - 1 - r*sin(angleRadians + PI), GxEPD_BLACK);
        }
        // Flèche du vent
        int xFleche = x + r*cos(angleRadians);
        int yFleche = y - r*sin(angleRadians);
        int rFleche = 15;
        display.drawLine(xFleche, yFleche, xFleche + rFleche*cos(5*PI/6 + angleRadians), yFleche - rFleche*sin(5*PI/6 + angleRadians), GxEPD_BLACK);
        display.drawLine(xFleche, yFleche, xFleche + rFleche*cos(7*PI/6 + angleRadians), yFleche - rFleche*sin(7*PI/6 + angleRadians), GxEPD_BLACK);
        if(cos(5*PI/6 + angleRadians) < 0.5) { 
          display.drawLine(xFleche - 1, yFleche, xFleche - 1 + rFleche*cos(5*PI/6 + angleRadians), yFleche - rFleche*sin(5*PI/6 + angleRadians), GxEPD_BLACK);
          display.drawLine(xFleche + 1, yFleche, xFleche + 1 + rFleche*cos(5*PI/6 + angleRadians), yFleche - rFleche*sin(5*PI/6 + angleRadians), GxEPD_BLACK);
        } else {
          display.drawLine(xFleche, yFleche - 1, xFleche + rFleche*cos(5*PI/6 + angleRadians), yFleche - 1 - rFleche*sin(5*PI/6 + angleRadians), GxEPD_BLACK);
          display.drawLine(xFleche, yFleche + 1, xFleche + rFleche*cos(5*PI/6 + angleRadians), yFleche + 1 - rFleche*sin(5*PI/6 + angleRadians), GxEPD_BLACK);
        }
        if(cos(7*PI/6 + angleRadians) < 0.5) { 
          display.drawLine(xFleche - 1, yFleche, xFleche - 1 + rFleche*cos(7*PI/6 + angleRadians), yFleche - rFleche*sin(7*PI/6 + angleRadians), GxEPD_BLACK);
          display.drawLine(xFleche + 1, yFleche, xFleche + 1 + rFleche*cos(7*PI/6 + angleRadians), yFleche - rFleche*sin(7*PI/6 + angleRadians), GxEPD_BLACK);
        } else {
          display.drawLine(xFleche, yFleche - 1, xFleche + rFleche*cos(7*PI/6 + angleRadians), yFleche - 1 - rFleche*sin(7*PI/6 + angleRadians), GxEPD_BLACK);
          display.drawLine(xFleche, yFleche + 1, xFleche + rFleche*cos(7*PI/6 + angleRadians), yFleche + 1 - rFleche*sin(7*PI/6 + angleRadians), GxEPD_BLACK);
        }
        /*if(cos(-angleRadians + 4*PI/3) < 0.5) { 
          display.drawLine(xFleche + 1, yFleche, xFleche + 1 + rFleche*cos(-angleRadians + 4*PI/3), yFleche - rFleche*sin(-angleRadians + 4*PI/3), GxEPD_BLACK);
          display.drawLine(xFleche - 1, yFleche, xFleche - 1 + rFleche*cos(-angleRadians + 4*PI/3), yFleche - rFleche*sin(-angleRadians + 4*PI/3), GxEPD_BLACK);
        } else {
          display.drawLine(xFleche, yFleche + 1, xFleche + rFleche*cos(-angleRadians + 4*PI/3), yFleche + 1 - rFleche*sin(-angleRadians + 4*PI/3), GxEPD_BLACK);
          display.drawLine(xFleche, yFleche - 1, xFleche + rFleche*cos(-angleRadians + 4*PI/3), yFleche - 1 - rFleche*sin(-angleRadians + 4*PI/3), GxEPD_BLACK);
        }*/

        //display.drawCircle(x, y, r+1, GxEPD_BLACK);
        //display.drawCircle(x, y, r+2, GxEPD_BLACK);
        //u8g2Fonts.print(weather.get_windDirection());

        //Sunrise - Sunset
        currentLine = currentLine + 25; 
        u8g2Fonts.setFont(u8g2_font_helvB10_tf);
        u8g2Fonts.setCursor(WeatherIconSize + 70, currentLine);
        u8g2Fonts.print(weather.get_sunrise());
        u8g2Fonts.setCursor(WeatherIconSize + 160, currentLine);
        u8g2Fonts.print(weather.get_sunset());

        display.setFont(&MonitoringWeatherIcon18pt7b);
        display.setCursor(WeatherIconSize + 40, currentLine);
        display.print("P");
        display.setCursor(WeatherIconSize + 130, currentLine);
        display.print("Q");

        //Air Pollution
        display.setFont(&MonitoringWeatherIcon18pt7b);
        display.setCursor(WeatherIconSize + 210, currentLine + 5);
        display.print(airPollution.get_airPollution("current"));
        
        if ( currentLine < TableauHaut ) { currentLine = TableauHaut; }
        currentLine = currentLine + 20;
        u8g2Fonts.setFont(u8g2_font_helvB10_tf);
        
        Serial.println("> Tableau");
        for (int t=0; t < weather.get_nbElementForecast(); t++)
        {
          u8g2Fonts.setCursor(15, currentLine);
          u8g2Fonts.print(weather.get_dateForecast(t));

          display.setFont(&MonitoringWeatherIcon18pt7b);
          display.setCursor(120, currentLine + 5);
          display.print(weather.get_situationFont(t));
          
          u8g2Fonts.setCursor(180, currentLine);
          u8g2Fonts.print(weather.get_temperatureAll("forecast", t));
          
          //u8g2Fonts.setCursor(260, currentLine);
          //u8g2Fonts.print(weather.get_pressure("forecast", t));
          
          u8g2Fonts.setCursor(310, currentLine);
          u8g2Fonts.print(weather.get_humidity("forecast", t));

          u8g2Fonts.setCursor(370, currentLine);
          u8g2Fonts.print(weather.get_windSpeed("forecast", t));

          display.setFont(&MonitoringWeatherIcon18pt7b); 
          display.setCursor(440, currentLine + 5);
          if (airPollution.get_airPollution("forecast", weather.get_dtForecast(t)) >= 0 && airPollution.get_airPollution("forecast", weather.get_dtForecast(t)) <= 5 )
          { display.print(airPollution.get_airPollution("forecast", weather.get_dtForecast(t))); }
          
          currentLine = currentLine + 30;
        }
      }

      Serial.println("> Dessin");

      // Weather
      if ( !statusDomoticz ) 
      {  u8g2Fonts.setCursor(zoneDessin_width / 2, zoneDessin_haut + 100); u8g2Fonts.print("No data"); }
      else
      {
        // Dessin de l'appart - on va utiliser des rectangles pour avoir des lignes plus épaisses !
        // DEPRECATED const char* dateJourHeure = dateSync.get_dateJourHeure();
        display.fillRect(zoneDessin_gauche, zoneDessin_haut, zoneDessin_width, zoneDessin_epaisseur, GxEPD_BLACK); // mur haut
        display.fillRect(zoneDessin_gauche, zoneDessin_haut, zoneDessin_epaisseur, zoneDessin_height, GxEPD_BLACK); // mur gauche
        display.fillRect(zoneDessin_gauche + zoneDessin_width, zoneDessin_haut, zoneDessin_epaisseur, zoneDessin_height, GxEPD_BLACK); //mur droite
        display.fillRect(zoneDessin_gauche, zoneDessin_haut + zoneDessin_height, zoneDessin_width + zoneDessin_epaisseur, zoneDessin_epaisseur, GxEPD_BLACK); //mur bas


        display.fillRect(zoneDessin_gauche + 160, zoneDessin_haut, zoneDessin_epaisseur, 110, GxEPD_BLACK); // chambre Clo
        display.fillRect(zoneDessin_gauche + 160 + 160, zoneDessin_haut, zoneDessin_epaisseur, 110, GxEPD_BLACK); // chambre E & M
        
        display.fillRect(zoneDessin_gauche + 390, zoneDessin_haut + 80, 40, zoneDessin_epaisseur, GxEPD_BLACK); // toilette haut
        display.fillRect(zoneDessin_gauche + 390, zoneDessin_haut + 80, zoneDessin_epaisseur, 30, GxEPD_BLACK); // toilette gauche

        display.fillRect(zoneDessin_gauche, zoneDessin_haut + 110, zoneDessin_width, zoneDessin_epaisseur, GxEPD_BLACK); // couloir haut
        display.fillRect(zoneDessin_gauche, zoneDessin_haut + 160, 320 + zoneDessin_epaisseur, zoneDessin_epaisseur, GxEPD_BLACK); // couloir bas
        display.fillRect(zoneDessin_gauche + 110, zoneDessin_haut + 110, zoneDessin_epaisseur, 50, GxEPD_BLACK); // salle d'eau / couloir gauche
        display.fillRect(zoneDessin_gauche + 320, zoneDessin_haut + 110, zoneDessin_epaisseur, 50, GxEPD_BLACK); // couloir droite
        display.fillRect(zoneDessin_gauche + 160, zoneDessin_haut + 160, zoneDessin_epaisseur, zoneDessin_height - 160, GxEPD_BLACK); // Chambre B & N

        // Dehors Jardin
        u8g2Fonts.setCursor(display_width / 2 - 50, zoneDessin_haut - 10);
        u8g2Fonts.print(domoticz.get_Sensor("Dehors Jardin", "Temp", true));
        u8g2Fonts.setCursor(display_width / 2 + 20, zoneDessin_haut - 10);
        u8g2Fonts.print(domoticz.get_Sensor("Dehors Jardin", "Humidity", true));

        // Dehors Rue
        u8g2Fonts.setCursor(display_width / 2 - 50, zoneDessin_haut + zoneDessin_height + 20);
        u8g2Fonts.print(domoticz.get_Sensor("Dehors Rue", "Temp", true));
        u8g2Fonts.setCursor(display_width / 2 + 20, zoneDessin_haut + zoneDessin_height + 20);
        u8g2Fonts.print(domoticz.get_Sensor("Dehors Rue", "Humidity", true));

        // Chambre Ben & Nat
        u8g2Fonts.setCursor(zoneDessin_gauche + 60, zoneDessin_haut + 190);
        u8g2Fonts.print(domoticz.get_Sensor("Chambre Ben & Nat", "Temp", false));
        u8g2Fonts.setCursor(zoneDessin_gauche + 60, zoneDessin_haut + 210);
        u8g2Fonts.print(domoticz.get_Sensor("Chambre Ben & Nat", "Humidity", false));

        // Chambre Clo
        u8g2Fonts.setCursor(zoneDessin_gauche + 60, zoneDessin_haut + 50);
        u8g2Fonts.print(domoticz.get_Sensor("Chambre Clo", "Temp", false));
        u8g2Fonts.setCursor(zoneDessin_gauche + 60, zoneDessin_haut + 70);
        u8g2Fonts.print(domoticz.get_Sensor("Chambre Clo", "Humidity", false));

        // Chambre Elé & Mayo
        u8g2Fonts.setCursor(zoneDessin_gauche + 210, zoneDessin_haut + 50);
        u8g2Fonts.print(domoticz.get_Sensor("Chambre Elé & Mayo", "Temp", false));
        u8g2Fonts.setCursor(zoneDessin_gauche + 210, zoneDessin_haut + 70);
        u8g2Fonts.print(domoticz.get_Sensor("Chambre Elé & Mayo", "Humidity", false));

        // Salon
        u8g2Fonts.setCursor(zoneDessin_gauche + 240, zoneDessin_haut + 190);
        u8g2Fonts.print(domoticz.get_Sensor("Salon", "Temp", false));
        u8g2Fonts.setCursor(zoneDessin_gauche + 240, zoneDessin_haut + 210);
        u8g2Fonts.print(domoticz.get_Sensor("Salon", "Humidity", false));

        // Salle de bains
        u8g2Fonts.setCursor(zoneDessin_gauche + 360, zoneDessin_haut + 50);
        u8g2Fonts.print(domoticz.get_Sensor("Salle de bains", "Temp", false));
        u8g2Fonts.setCursor(zoneDessin_gauche + 360, zoneDessin_haut + 70);
        u8g2Fonts.print(domoticz.get_Sensor("Salle de bains", "Humidity", false));

        // Salle d'eau
        u8g2Fonts.setCursor(zoneDessin_gauche + 40, zoneDessin_haut + 130);
        u8g2Fonts.print(domoticz.get_Sensor("Salle d'eau", "Temp", false));
        u8g2Fonts.setCursor(zoneDessin_gauche + 40, zoneDessin_haut + 150);
        u8g2Fonts.print(domoticz.get_Sensor("Salle d'eau", "Humidity", false));

        // Placard NAS
        u8g2Fonts.setCursor(zoneDessin_gauche + 210, zoneDessin_haut + 140);
        u8g2Fonts.print(domoticz.get_Sensor("Placard NAS", "Temp", false));
      }

      // Footer
      Serial.println("> Footer");
      currentLine = display_height - 10;

      u8g2Fonts.setCursor(0, currentLine);
      if ( statusDateSync ) { u8g2Fonts.print(get_dateHeure(&timeinfo)); }

      u8g2Fonts.setCursor(display_width - 120, currentLine);
      u8g2Fonts.print(battery_level());

    } while (display.nextPage());

    //display.setPartialWindow(display_height - WeatherIconSize, 0, WeatherIconSize, WeatherIconSize);
    if ( statusWeather ) { display.drawImage(weather.get_situationIcon(), display_height - WeatherIconSize, 0, WeatherIconSize, WeatherIconSize, false, false, true); }
    
  }

  /// RAZ écran ///

  //display.clearScreen();
  display.hibernate();
  display.powerOff();
  display.end();

  Serial.println("#FIN BOUCLE");

  //delay(10000);
}

void loop() {
  //Nothing because in deep sleep mode, the esp32 restart
}