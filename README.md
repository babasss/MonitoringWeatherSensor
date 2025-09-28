MonitoringWeatherSensor
Un projet Arduino pour surveiller et afficher des informations météorologiques à partir de capteurs et d'API externes. Il inclut la gestion de la batterie, la connexion Wi-Fi et des icônes météo personnalisées pour l'affichage.

🛠️ Fonctionnalités
Lecture des capteurs météo (température, humidité, pression, etc.).
Connexion à Internet via Wi-Fi.
Intégration de plusieurs API :
Météo
Pollution de l’air
Dates et anniversaires
Domoticz
Nominis
Affichage d’icônes météo personnalisées.
Gestion de la batterie.

📁 Structure du projet
Fichier	Description
MonitoringWeatherSensor.ino	Fichier principal du projet Arduino.
WeatherIcons*.h	Bibliothèques d’icônes météo pour l’affichage.
api_*.h	Gestion des appels aux différentes API externes.
battery.h	Gestion de la batterie et de l’alimentation.
wifi_connect.h	Gestion de la connexion Wi-Fi.
config.h	Paramètres de configuration du projet.
date.h	Gestion des dates et calendrier.
functions.h	Fonctions auxiliaires diverses.
⚙️ Installation

Installer Arduino IDE
 ou PlatformIO
.
Cloner le dépôt : git clone https://github.com/babasss/MonitoringWeatherSensor.git
Installer les bibliothèques Arduino nécessaires (LiquidCrystal, WiFi, etc.).
Configurer config.h avec vos paramètres Wi-Fi et API.
Ouvrir MonitoringWeatherSensor.ino dans l’IDE et téléverser sur votre carte Arduino.

🚀 Utilisation
Allumer l’Arduino et vérifier que la connexion Wi-Fi est établie.
Les données météo et autres informations s’affichent sur l’écran LCD.
Les icônes météo sont affichées en fonction des conditions actuelles.
