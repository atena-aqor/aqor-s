# Documentation de la classe SDS011Sensor

## Description
La classe `SDS011Sensor` est une interface pour le capteur de particules fines SDS011, destinée à être utilisée avec les microcontrôleurs ESP32, en particulier le Heltec Wireless Stick Lite (V3). Elle gère l'alimentation du capteur, la communication série et le traitement des données pour mesurer les concentrations de particules PM2.5 et PM10 dans l'air.

## Caractéristiques
- Contrôle de l'alimentation pour économiser l'énergie
- Communication série via UART
- Calcul des moyennes de mesures pour améliorer la précision
- Formatage des données en JSON avec catégorisation de la qualité de l'air

## Configuration matérielle
- **RX** : GPIO5 pour recevoir les données du capteur
- **TX** : GPIO4 pour envoyer des commandes au capteur
- **Alimentation** : GPIO21 pour contrôler l'alimentation du capteur

## Méthodes

### Constructeur
```cpp
SDS011Sensor(int rxPin, int txPin, int powerPin);
```
Initialise le capteur avec les broches définies.
- `rxPin` : Broche de réception des données (GPIO5 recommandé)
- `txPin` : Broche de transmission des données (GPIO4 recommandé)
- `powerPin` : Broche contrôlant l'alimentation du capteur (GPIO21 recommandé)

### begin()
```cpp
void begin();
```
Configure les broches et l'UART pour la communication. Éteint le capteur par défaut.

### powerOn()
```cpp
void powerOn();
```
Active l'alimentation du capteur en mettant la broche d'alimentation à l'état HAUT.

### powerOff()
```cpp
void powerOff();
```
Désactive l'alimentation du capteur en mettant la broche d'alimentation à l'état BAS.

### readSensor()
```cpp
bool readSensor(float& pm25, float& pm10);
```
Lit une mesure unique du capteur si des données sont disponibles.
- `pm25` : Variable où stocker la valeur PM2.5 (en µg/m³)
- `pm10` : Variable où stocker la valeur PM10 (en µg/m³)
- **Retourne** : `true` si une mesure a été lue avec succès, `false` sinon

### measure()
```cpp
bool measure(float& pm25, float& pm10, int warmupTime = 10000, int sampleTime = 5000);
```
Réalise un cycle complet de mesure avec allumage du capteur, préchauffage, mesures multiples et extinction.
- `pm25` : Variable où stocker la valeur moyenne PM2.5 (en µg/m³)
- `pm10` : Variable où stocker la valeur moyenne PM10 (en µg/m³)
- `warmupTime` : Temps de préchauffage en millisecondes (défaut : 10000)
- `sampleTime` : Temps d'échantillonnage en millisecondes (défaut : 5000)
- **Retourne** : `true` si des mesures ont été obtenues avec succès, `false` sinon

### getJSON()
```cpp
String getJSON(float pm25, float pm10);
```
Génère une chaîne au format JSON contenant les données de mesure.
- `pm25` : Valeur PM2.5 à inclure (en µg/m³)
- `pm10` : Valeur PM10 à inclure (en µg/m³)
- **Retourne** : Chaîne JSON formatée

## Exemple d'utilisation

```cpp
#include "SDS011Sensor.h"

// Configuration des broches
#define RX_PIN 5     // GPIO5 pour RXD
#define TX_PIN 4     // GPIO4 pour TXD
#define POWER_PIN 21 // GPIO21 pour alimenter le capteur

SDS011Sensor particleSensor(RX_PIN, TX_PIN, POWER_PIN);

void setup() {
    Serial.begin(115200);
    particleSensor.begin();
    Serial.println("Capteur SDS011 initialisé");
}

void loop() {
    float pm25, pm10;
    
    if (particleSensor.measure(pm25, pm10)) {
        String jsonData = particleSensor.getJSON(pm25, pm10);
        Serial.println(jsonData);
    } else {
        Serial.println("{\"error\":\"Échec de la mesure\"}");
    }
    
    delay(300000); // 5 minutes entre les mesures
}
```

## Format JSON

Le format JSON retourné par `getJSON()` contient les champs suivants :
- `pm25` : Concentration de particules PM2.5 en µg/m³
- `pm10` : Concentration de particules PM10 en µg/m³
- `timestamp` : Horodatage en millisecondes depuis le démarrage du microcontrôleur
- `pm25_category` : Catégorie de qualité de l'air pour PM2.5 (Très bon, Bon, Moyen, Médiocre, Mauvais)
- `pm10_category` : Catégorie de qualité de l'air pour PM10 (Très bon, Bon, Moyen, Médiocre, Mauvais)

Exemple de sortie JSON :
```json
{
  "pm25": 12.3,
  "pm10": 23.4,
  "timestamp": 3600000,
  "pm25_category": "Bon",
  "pm10_category": "Très bon"
}
```

## Notes techniques
- Le capteur nécessite un préchauffage pour des mesures précises (10 secondes par défaut)
- Pour économiser l'énergie et prolonger la durée de vie du capteur, il est éteint entre les mesures
- Les catégories de qualité de l'air sont basées sur les normes européennes
