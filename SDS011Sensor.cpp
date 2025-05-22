#include "SDS011Sensor.h"

// Constructeur avec la syntaxe correcte
SDS011Sensor::SDS011Sensor(int rxPin, int txPin, int powerPin) 
    : _sdsSerial(1), _rxPin(rxPin), _txPin(txPin), _powerPin(powerPin) {
}

void SDS011Sensor::begin() {
    _sdsSerial.begin(9600, SERIAL_8N1, _rxPin, _txPin);
    pinMode(_powerPin, OUTPUT);
    powerOff(); // Démarrer avec le capteur éteint
}

void SDS011Sensor::powerOn() {
    digitalWrite(_powerPin, HIGH);
}

void SDS011Sensor::powerOff() {
    digitalWrite(_powerPin, LOW);
}

bool SDS011Sensor::readSensor(float& pm25, float& pm10) {
    if (_sdsSerial.available() >= 10) {
        _sdsSerial.readBytes(_buffer, 10);
        
        // Vérifier l'en-tête et la fin de trame
        if (_buffer[0] == 0xAA && _buffer[1] == 0xC0 && _buffer[9] == 0xAB) {
            // Calculer PM2.5 et PM10
            pm25 = ((_buffer[3] * 256) + _buffer[2]) / 10.0;
            pm10 = ((_buffer[5] * 256) + _buffer[4]) / 10.0;
            return true;
        }
    }
    return false;
}

bool SDS011Sensor::measure(float& pm25, float& pm10, int warmupTime, int sampleTime) {
    bool readSuccess = false;
    
    // Allumer le capteur
    powerOn();
    
    // Attendre que le capteur se réchauffe
    delay(warmupTime);
    
    // Vider le buffer série
    while (_sdsSerial.available()) {
        _sdsSerial.read();
    }
    
    // Période de mesure
    unsigned long startTime = millis();
    float totalPM25 = 0;
    float totalPM10 = 0;
    int readCount = 0;
    
    while (millis() - startTime < sampleTime) {
        float tempPM25, tempPM10;
        if (readSensor(tempPM25, tempPM10)) {
            totalPM25 += tempPM25;
            totalPM10 += tempPM10;
            readCount++;
        }
        delay(100);
    }
    
    // Éteindre le capteur après la mesure
    powerOff();
    
    // Calculer la moyenne si des lectures ont été obtenues
    if (readCount > 0) {
        pm25 = totalPM25 / readCount;
        pm10 = totalPM10 / readCount;
        readSuccess = true;
    }
    
    return readSuccess;
}

String SDS011Sensor::getJSON(float pm25, float pm10) {
    // Créer une chaîne JSON avec les données du capteur
    String jsonString = "{";
    jsonString += "\"Sensor\": \"SDS011\" ,";
    jsonString += "\"pm25\":" + String(pm25, 1) + ","; // 1 décimale
    jsonString += "\"pm10\":" + String(pm10, 1) ;
    
    /*// Ajouter un timestamp (millisecondes depuis le démarrage)
    jsonString += "\"timestamp\":" + String(millis()) + ",";
    
    // Ajouter les catégories de qualité de l'air selon les normes européennes
    // PM2.5 catégories: 0-10 (Très bon), 10-20 (Bon), 20-25 (Moyen), 25-50 (Médiocre), 50+ (Mauvais)
    String pm25Category;
    if (pm25 <= 10) pm25Category = "\"Très bon\"";
    else if (pm25 <= 20) pm25Category = "\"Bon\"";
    else if (pm25 <= 25) pm25Category = "\"Moyen\"";
    else if (pm25 <= 50) pm25Category = "\"Médiocre\"";
    else pm25Category = "\"Mauvais\"";
    
    // PM10 catégories: 0-20 (Très bon), 20-40 (Bon), 40-50 (Moyen), 50-100 (Médiocre), 100+ (Mauvais)
    String pm10Category;
    if (pm10 <= 20) pm10Category = "\"Très bon\"";
    else if (pm10 <= 40) pm10Category = "\"Bon\"";
    else if (pm10 <= 50) pm10Category = "\"Moyen\"";
    else if (pm10 <= 100) pm10Category = "\"Médiocre\"";
    else pm10Category = "\"Mauvais\"";
    
    jsonString += "\"pm25_category\":" + pm25Category + ",";
    jsonString += "\"pm10_category\":" + pm10Category;*/
    
    jsonString += "}";
    return jsonString;
}
