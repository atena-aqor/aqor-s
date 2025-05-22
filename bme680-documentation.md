# Documentation de la classe BME680Manager

## Description
La classe `BME680Manager` est une classe héritée de `Bsec` qui permet de gérer le capteur environnemental BME680 de Bosch. Elle utilise le protocole I2C pour communiquer avec le capteur et fournit des fonctions pour la configuration, la lecture des données et la mise en veille du capteur.

## Caractéristiques
- Lecture des données de température, humidité, pression atmosphérique et résistance de gaz
- Calcul de l'indice de qualité de l'air (IAQ)
- Estimation des équivalents de CO2 et de COV
- Mode veille pour économiser l'énergie
- Sortie de données au format JSON

## Dépendances
- Arduino.h
- Wire.h
- bsec.h (bibliothèque BSEC de Bosch)
- ArduinoJson.h

## Configuration matérielle
- Microcontrôleur: Heltec WiFi Lora 32 Stick Lite
- Communication I2C:
  - SDA: GPIO41
  - SCL: GPIO42
- Adresse I2C par défaut: 0x76 (peut être 0x77 sur certains capteurs)

## API

### Constructeur
```cpp
BME680Manager()
```
Crée une instance de la classe BME680Manager.

### Méthodes publiques

#### begin()
```cpp
bool begin()
```
Initialise le capteur BME680 et configure les capteurs virtuels BSEC.

**Retour:**
- `true` si l'initialisation a réussi
- `false` en cas d'échec

**Exemple:**
```cpp
if (!bme680.begin()) {
    Serial.println("Échec de l'initialisation du capteur BME680");
}
```

#### sleep()
```cpp
void sleep()
```
Met le capteur en mode veille pour économiser l'énergie.

**Exemple:**
```cpp
bme680.sleep();
```

#### wakeUp()
```cpp
void wakeUp()
```
Réveille le capteur du mode veille et le réinitialise.

**Exemple:**
```cpp
bme680.wakeUp();
```

#### readData()
```cpp
bool readData()
```
Lit les données du capteur et met à jour les variables internes.

**Retour:**
- `true` si de nouvelles données ont été lues
- `false` si aucune nouvelle donnée n'est disponible

**Exemple:**
```cpp
if (bme680.readData()) {
    // Traitement des nouvelles données
}
```

#### getJsonData()
```cpp
String getJsonData()
```
Retourne les valeurs des capteurs au format JSON.

**Retour:**
Une chaîne de caractères contenant les données au format JSON

**Format JSON:**
```json
{
    "temperature": 25.1,
    "humidity": 45.2,
    "pressure": 1013.25,
    "gas_resistance": 12345.67,
    "iaq": 50.5,
    "iaq_accuracy": 3,
    "co2_equivalent": 500.0,
    "voc_equivalent": 1.2
}
```

**Exemple:**
```cpp
String json = bme680.getJsonData();
Serial.println(json);
```

## Variables internes

### Capteurs disponibles
- **temperature**: Température en °C
- **humidity**: Humidité relative en %
- **pressure**: Pression atmosphérique en hPa
- **gasResistance**: Résistance du capteur de gaz en Ohms
- **iaq**: Indice de qualité de l'air (0-500)
- **iaqAccuracy**: Précision de l'IAQ (0-3)
- **co2Equivalent**: Estimation de la concentration en CO2 en ppm
- **breathVocEquivalent**: Estimation des composés organiques volatils en ppm

## Exemple d'utilisation complet

```cpp
#include <Arduino.h>
#include "BME680Manager.h"

BME680Manager bme680;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("Initialisation du capteur BME680...");
    if (!bme680.begin()) {
        Serial.println("Échec de l'initialisation du capteur BME680");
        while (1) delay(10);
    }
}

void loop() {
    // Lecture des données du capteur
    if (bme680.readData()) {
        // Affichage des données au format JSON
        Serial.println(bme680.getJsonData());
    }
    
    delay(5000);  // Attendre 5 secondes entre les lectures
    
    // Exemple de mise en veille
    bme680.sleep();
    delay(10000);  // En veille pendant 10 secondes
    bme680.wakeUp();
    delay(1000);   // Attendre que le capteur se stabilise
}
```

## Notes importantes
1. La bibliothèque BSEC doit être installée. Elle n'est pas disponible dans le gestionnaire de bibliothèques standard d'Arduino et doit être téléchargée depuis le site de Bosch.
2. Le capteur BME680 nécessite un temps de préchauffage pour stabiliser ses mesures, en particulier pour l'IAQ. Une précision optimale est atteinte après plusieurs heures de fonctionnement.
3. L'IAQ est un indice qui va de 0 (air très propre) à 500 (air très pollué).
4. La précision de l'IAQ est indiquée par la valeur iaqAccuracy:
   - 0 : Initialisation
   - 1 : Instable
   - 2 : Calibration en cours
   - 3 : Calibré (précision maximale)

## Dépannage
- Si le capteur ne s'initialise pas, vérifiez l'adresse I2C (0x76 ou 0x77)
- Si les valeurs sont incorrectes, vérifiez les connexions I2C et assurez-vous que le capteur n'est pas perturbé par des sources de chaleur ou d'humidité externes
- La bibliothèque BSEC étant propriétaire, elle peut nécessiter des mises à jour régulières depuis le site de Bosch

## Limitations
- Le mode veille implémenté dans cette classe est basique. Pour des économies d'énergie plus avancées, consultez la documentation BSEC de Bosch
- La fonction de calibration automatique de l'IAQ nécessite un fonctionnement continu du capteur sur plusieurs jours pour atteindre une précision optimale
