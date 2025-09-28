# MonitoringWeatherSensor

Un projet Arduino pour surveiller et afficher des informations météorologiques à partir de capteurs et d'API externes. Le projet inclut la gestion de la batterie, la connexion Wi-Fi et des icônes météo personnalisées pour l'affichage.

---

## 🛠️ Fonctionnalités

- Lecture des capteurs météo (température, humidité, pression, etc.).
- Connexion à Internet via Wi-Fi.
- Intégration de plusieurs API :
  - Météo
  - Pollution de l’air
  - Dates et anniversaires
  - Domoticz
  - Nominis
- Affichage d’icônes météo personnalisées.
- Gestion de la batterie et de l’alimentation.

---

## 📁 Structure du projet

| Fichier | Description |
|---------|------------|
| `MonitoringWeatherSensor.ino` | Fichier principal du projet Arduino. |
| `WeatherIcons*.h` | Bibliothèques d’icônes météo pour l’affichage. |
| `api_*.h` | Gestion des appels aux différentes API externes. |
| `battery.h` | Gestion de la batterie et de l’alimentation. |
| `wifi_connect.h` | Gestion de la connexion Wi-Fi. |
| `config.h` | Paramètres de configuration du projet. |
| `date.h` | Gestion des dates et calendrier. |
| `functions.h` | Fonctions auxiliaires diverses. |

---

## ⚙️ Installation

1. Installer [Arduino IDE](https://www.arduino.cc/en/software) ou [PlatformIO](https://platformio.org/).
2. Cloner le dépôt :
   ```bash
   git clone https://github.com/babasss/MonitoringWeatherSensor.git
