// ESP32
const int time_to_sleep = 1800; // temps de sommeil en secondes (1800 = 30min)

// API
const int maxRetries = 5; // nombre d'essai pour joindre l'API
const int retryPause = 10000;

// Change to your WiFi credentials
const char ssid[] = "";     // WiFi SSID to connect to
const char ssid_password[] = ""; // WiFi password needed for the SSID

// Use your own API key by signing up for a free developer account at https://openweathermap.org/
const char weather_apikey[] = "";                      // See: https://openweathermap.org/  // It's free to get an API key, but don't take more than 60 readings/minute!
const char weather_lang[] = "fr";
const char weather_lat[] = "48.8534";
const char weather_long[] = "2.3488";
const char weather_unit[] = "metric";
const char weather_url[] = "http://api.openweathermap.org";

const char nominis_url[] = "https://nominis.cef.fr/json/saintdujour.php";

const char date_url[] = "https://www.timeapi.io/api/timezone/zone?timeZone=Europe/Paris";

// Domoticz
const char domoticz_apikey[] = "";
const char domoticz_url[] = "http://X.X.X.X/json.htm?type=devices&used=true&order=Name";

// Birthday Calendar
const char birthday_url[] = "https://calendar.google.com/calendar/ical/n3jl5cv82ftlg8208ftkd2086g%40group.calendar.google.com/private-4818f33937911c10f8005373bb85a72b/basic.ics";