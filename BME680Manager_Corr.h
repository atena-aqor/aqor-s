#ifndef BME680_MANAGER_H
#define BME680_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <bsec.h>
#include <ArduinoJson.h>

class BME680Manager:  public Bsec {
private:
    const uint8_t i2c_sda = 41;
    const uint8_t i2c_scl = 42;
    const uint8_t bme680_address = 0x77;  //Adresse I2C par défaut, vérifiez celle de votre capteur
    
     //Variables pour stocker les valeurs compensées du capteur
    float temperature;
    float humidity;
    float pressure;
    float gasResistance;
    float iaq;            //Index de qualité de l'air
    float iaqAccuracy;    //Précision de l'IAQ
    float co2Equivalent;
    float breathVocEquivalent;
    float stabStatus;     //Statut de stabilisation
    float runInStatus;    //Statut de calibration
    
//    Fonction pour arrondir à une décimale
    float roundToOneDecimal(float value) {
        return round(value * 10.0)/  10.0;
    }
    
public:
     //Énumération pour identifier les différentes valeurs
    enum BME680Value {
        BME680_TEMPERATURE = 0,
        BME680_HUMIDITY = 1,
        BME680_PRESSURE = 2,
        BME680_GAS_RESISTANCE = 3,
        BME680_IAQ = 4,
        BME680_IAQ_ACCURACY = 5,
        BME680_CO2_EQUIVALENT = 6,
        BME680_VOC_EQUIVALENT = 7,
        BME680_STAB_STATUS = 8,
        BME680_RUN_IN_STATUS = 9
    };
    
    BME680Manager() {}
    
    bool begin() {
         //Initialiser la communication I2C avec les broches spécifiées
        Wire.begin(i2c_sda, i2c_scl);
        
         //Initialiser le capteur BSEC
        if (!Bsecbegin(bme680_address, Wire)) {
            Serial.println("Erreur d'initialisation du capteur BME680");
            return false;
        }
        
         //Configuration des capteurs virtuels BSEC avec valeurs compensées
        bsec_virtual_sensor_t sensorList[] = {
             //Valeurs brutes pour les calculs internes de BSEC
            BSEC_OUTPUT_RAW_TEMPERATURE,
            BSEC_OUTPUT_RAW_PRESSURE,
            BSEC_OUTPUT_RAW_HUMIDITY,
            BSEC_OUTPUT_RAW_GAS,
            
             //Valeurs compensées (corrigées) que nous utiliserons
            BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,   //Température compensée
            BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,      //Humidité compensée
            BSEC_OUTPUT_COMPENSATED_GAS,                      //Résistance de gaz compensée
            
             //Valeurs IAQ et dérivées
            BSEC_OUTPUT_IAQ,                                  //Indice de qualité de l'air
            BSEC_OUTPUT_STATIC_IAQ,                           //IAQ statique
            BSEC_OUTPUT_CO2_EQUIVALENT,                       //Équivalent CO2
            BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,                //Équivalent COV
            
             //Statuts
            BSEC_OUTPUT_STABILIZATION_STATUS,                 //Statut de stabilisation
            BSEC_OUTPUT_RUN_IN_STATUS                         //Statut de calibration
        };
        
         //Définir la fréquence de mise à jour des capteurs (3.3V, fréquence en secondes)
        BsecupdateSubscription(sensorList, sizeof(sensorList)  sizeof(sensorList[0]), BSEC_SAMPLE_RATE_LP);
        
        Serial.println("BME680 initialisé avec succès");
        return true;
    }
    
    void sleep() {
         //Mettre le capteur en mode veille
        Wire.beginTransmission(bme680_address);
        Wire.write(0x74);  //Registre ctrl_meas
        Wire.write(0x00);  //Mettre en mode sleep (0b00000000)
        Wire.endTransmission();
        Serial.println("BME680 mis en veille");
    }
    
    void wakeUp() {
         //Réveiller le capteur (mode normal)
        begin();
        Serial.println("BME680 réveillé");
    }
    
    bool readData() {
         //Vérifier si de nouvelles données sont disponibles
        if (Bsecrun()) {
             //Utiliser les valeurs compensées plutôt que les valeurs brutes
            temperature = Bsectemperature;        //Température compensée
            humidity = Bsechumidity;              //Humidité compensée
            pressure = Bsecpressure  100.0;      //Conversion en hPa
            gasResistance = BsecgasResistance;    //Résistance de gaz compensée
            
             //Valeurs IAQ
            iaq = Bseciaq;
            iaqAccuracy = BseciaqAccuracy;
            co2Equivalent = Bsecco2Equivalent;
            breathVocEquivalent = BsecbreathVocEquivalent;
            
             //Statuts additionnels
            stabStatus = BsecstabStatus;
            runInStatus = BsecrunInStatus;
            
            return true;
        }
        return false;
    }
    
     //Récupérer une valeur spécifique (avec option d'arrondi)
    float getDataValue(BME680Value valueType, bool rounded = false) {
        float value = 0.0;
        
        switch(valueType) {
            case BME680_TEMPERATURE
                value = temperature;
                break;
            case BME680_HUMIDITY
                value = humidity;
                break;
            case BME680_PRESSURE
                value = pressure;
                break;
            case BME680_GAS_RESISTANCE
                value = gasResistance;
                break;
            case BME680_IAQ
                value = iaq;
                break;
            case BME680_IAQ_ACCURACY
                value = iaqAccuracy;
                break;
            case BME680_CO2_EQUIVALENT
                value = co2Equivalent;
                break;
            case BME680_VOC_EQUIVALENT
                value = breathVocEquivalent;
                break;
            case BME680_STAB_STATUS
                value = stabStatus;
                break;
            case BME680_RUN_IN_STATUS
                value = runInStatus;
                break;
        }
        
        if (rounded) {
            return roundToOneDecimal(value);
        }
        
        return value;
    }
    
     //Récupérer toutes les valeurs dans un tableau (avec option d'arrondi)
     //Note le tableau doit être d'au moins 10 éléments
    void getDataValues(float values[10], bool rounded = false) {
        values[BME680_TEMPERATURE] = temperature;
        values[BME680_HUMIDITY] = humidity;
        values[BME680_PRESSURE] = pressure;
        values[BME680_GAS_RESISTANCE] = gasResistance;
        values[BME680_IAQ] = iaq;
        values[BME680_IAQ_ACCURACY] = iaqAccuracy;
        values[BME680_CO2_EQUIVALENT] = co2Equivalent;
        values[BME680_VOC_EQUIVALENT] = breathVocEquivalent;
        values[BME680_STAB_STATUS] = stabStatus;
        values[BME680_RUN_IN_STATUS] = runInStatus;
        
        if (rounded) {
            for (int i = 0; i  10; i++) {
                 //Ne pas arrondir les valeurs qui sont des indicateurs d'état
                if (i != BME680_IAQ_ACCURACY && i != BME680_STAB_STATUS && i != BME680_RUN_IN_STATUS) {
                    values[i] = roundToOneDecimal(values[i]);
                }
            }
        }
    }
    
    String getJsonData(bool rounded = false) {
         //Créer un objet JSON avec les données compensées du capteur
        StaticJsonDocument512 jsonDoc;
        
        jsonDoc[temperature] = rounded  roundToOneDecimal(temperature)  temperature;
        jsonDoc[humidity] = rounded  roundToOneDecimal(humidity)  humidity;
        jsonDoc[pressure] = rounded  roundToOneDecimal(pressure)  pressure;
        jsonDoc[gas_resistance] = rounded  roundToOneDecimal(gasResistance)  gasResistance;
        jsonDoc[iaq] = rounded  roundToOneDecimal(iaq)  iaq;
        jsonDoc[iaq_accuracy] = (int)iaqAccuracy;  Toujours en entier
        jsonDoc[co2_equivalent] = rounded  roundToOneDecimal(co2Equivalent)  co2Equivalent;
        jsonDoc[voc_equivalent] = rounded  roundToOneDecimal(breathVocEquivalent)  breathVocEquivalent;
        jsonDoc[stabilization_status] = (int)stabStatus;
        jsonDoc[run_in_status] = (int)runInStatus;
        
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        
        return jsonString;
    }
};

#endif  BME680_MANAGER_H
