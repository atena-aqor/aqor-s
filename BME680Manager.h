#ifndef BME680_MANAGER_H
#define BME680_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <bsec.h>
#include <ArduinoJson.h>

class BME680Manager : public Bsec {
private:
    const uint8_t i2c_sda = 41;
    const uint8_t i2c_scl = 42;
    const uint8_t bme680_address = 0x77; // Adresse I2C par défaut, vérifiez celle de votre capteur
    
    // Variables pour stocker les valeurs du capteur
    float temperature;
    float humidity;
    float pressure;
    float gasResistance;
    float iaq;           // Index de qualité de l'air
    float iaqAccuracy;   // Précision de l'IAQ
    float co2Equivalent;
    float breathVocEquivalent;
    
    // Fonction pour arrondir à une décimale
    float roundToOneDecimal(float value) {
        return round(value * 10.0) / 10.0;
    }
    
public:
    // Énumération pour identifier les différentes valeurs
    enum BME680Value {
        BME680_TEMPERATURE = 0,
        BME680_HUMIDITY = 1,
        BME680_PRESSURE = 2,
        BME680_GAS_RESISTANCE = 3,
        BME680_IAQ = 4,
        BME680_IAQ_ACCURACY = 5,
        BME680_CO2_EQUIVALENT = 6,
        BME680_VOC_EQUIVALENT = 7
    };
    
    BME680Manager() {}
    
    bool begin() {
        // Initialiser la communication I2C avec les broches spécifiées
        Wire.begin(i2c_sda, i2c_scl);
        Bsec::begin(bme680_address, Wire);
        
        // Initialiser le capteur BSEC
        if (Bsec::bsecStatus != BSEC_OK) {
            Serial.println("Erreur d'initialisation du capteur BME680");
            return false;
        }
        
        // Configuration des capteurs virtuels BSEC
        bsec_virtual_sensor_t sensorList[] = {
            BSEC_OUTPUT_RAW_TEMPERATURE,
            BSEC_OUTPUT_RAW_PRESSURE,
            BSEC_OUTPUT_RAW_HUMIDITY,
            BSEC_OUTPUT_RAW_GAS,
            BSEC_OUTPUT_IAQ,
            BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
            BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
            BSEC_OUTPUT_CO2_EQUIVALENT,
            BSEC_OUTPUT_BREATH_VOC_EQUIVALENT
        };
        
        // Définir la fréquence de mise à jour des capteurs (3.3V, fréquence en secondes)
        Bsec::updateSubscription(sensorList, sizeof(sensorList) / sizeof(sensorList[0]), BSEC_SAMPLE_RATE_LP);
        
        Serial.println("BME680 initialisé avec succès");
        return true;
    }
    
    void sleep() {
        // Mettre le capteur en mode veille
        Wire.beginTransmission(bme680_address);
        Wire.write(0x74); // Registre ctrl_meas
        Wire.write(0x00); // Mettre en mode sleep (0b00000000)
        Wire.endTransmission();
        Serial.println("BME680 mis en veille");
    }
    
    void wakeUp() {
        // Réveiller le capteur (mode normal)
        begin();
        Serial.println("BME680 réveillé");
    }
    
    bool readData() {
        // Vérifier si de nouvelles données sont disponibles
        if (Bsec::run()) {
            temperature = Bsec::temperature;
            humidity = Bsec::humidity;
            pressure = Bsec::pressure / 100.0; // Conversion en hPa
            gasResistance = Bsec::gasResistance;
            iaq = Bsec::iaq;
            iaqAccuracy = Bsec::iaqAccuracy;
            co2Equivalent = Bsec::co2Equivalent;
            breathVocEquivalent = Bsec::breathVocEquivalent;
            return true;
        }
        return false;
    }
    
    // Récupérer une valeur spécifique (avec option d'arrondi)
    float getDataValue(BME680Value valueType, bool rounded = false) {
        float value = 0.0;
        
        switch(valueType) {
            case BME680_TEMPERATURE:
                value = temperature;
                break;
            case BME680_HUMIDITY:
                value = humidity;
                break;
            case BME680_PRESSURE:
                value = pressure;
                break;
            case BME680_GAS_RESISTANCE:
                value = gasResistance;
                break;
            case BME680_IAQ:
                value = iaq;
                break;
            case BME680_IAQ_ACCURACY:
                value = iaqAccuracy;
                break;
            case BME680_CO2_EQUIVALENT:
                value = co2Equivalent;
                break;
            case BME680_VOC_EQUIVALENT:
                value = breathVocEquivalent;
                break;
        }
        
        if (rounded) {
            return roundToOneDecimal(value);
        }
        
        return value;
    }
    
    // Récupérer toutes les valeurs dans un tableau (avec option d'arrondi)
    void getDataValues(float values[8], bool rounded = false) {
        values[BME680_TEMPERATURE] = temperature;
        values[BME680_HUMIDITY] = humidity;
        values[BME680_PRESSURE] = pressure;
        values[BME680_GAS_RESISTANCE] = gasResistance;
        values[BME680_IAQ] = iaq;
        values[BME680_IAQ_ACCURACY] = iaqAccuracy;
        values[BME680_CO2_EQUIVALENT] = co2Equivalent;
        values[BME680_VOC_EQUIVALENT] = breathVocEquivalent;
        
        if (rounded) {
            for (int i = 0; i < 8; i++) {
                if (i != BME680_IAQ_ACCURACY) { // Ne pas arrondir l'accuracy qui est déjà un entier
                    values[i] = roundToOneDecimal(values[i]);
                }
            }
        }
    }
    
    String getJsonData(bool rounded = false) {
        // Créer un objet JSON avec les données du capteur
        StaticJsonDocument<512> jsonDoc;
        
        jsonDoc["temperature"] = rounded ? roundToOneDecimal(temperature) : temperature;
        jsonDoc["humidity"] = rounded ? roundToOneDecimal(humidity) : humidity;
        jsonDoc["pressure"] = rounded ? roundToOneDecimal(pressure) : pressure;
        jsonDoc["gas_resistance"] = rounded ? roundToOneDecimal(gasResistance) : gasResistance;
        jsonDoc["iaq"] = rounded ? roundToOneDecimal(iaq) : iaq;
        jsonDoc["iaq_accuracy"] = (int)iaqAccuracy; // Toujours en entier
        jsonDoc["co2_equivalent"] = rounded ? roundToOneDecimal(co2Equivalent) : co2Equivalent;
        jsonDoc["voc_equivalent"] = rounded ? roundToOneDecimal(breathVocEquivalent) : breathVocEquivalent;
        
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        
        return jsonString;
    }
};

#endif // BME680_MANAGER_H