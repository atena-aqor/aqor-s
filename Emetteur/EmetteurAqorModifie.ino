/*
 * EmetteurAQOR - Système de mesure de qualité d'air
 * 
 * Ce programme collecte des données environnementales (particules fines, température,
 * humidité, pression, COV, niveau sonore) et les transmet via LoRa.
 * Le système fonctionne en cycles de veille/réveil pour économiser l'énergie.
 */

#include "config.h"
#include "lora_manager.h"
#include "power_manager.h"
#include <SPI.h>

// Inclusion des gestionnaires de capteurs
#include "SDS011Sensor.h"
#include "BME680Manager.h"
#include "INMP441Manager.h"
#include "BatteryMonitor.h"

// Variables pour stocker les données mesurées entre les cycles de veille
RTC_DATA_ATTR int wake_count = 0;
RTC_DATA_ATTR float dBMoyen = 0;
RTC_DATA_ATTR float dBmax = 0;
RTC_DATA_ATTR int dBmaxCount = 0;

// Instanciation des objets pour les capteurs
SDS011Sensor particleSensor(RX_PIN, TX_PIN, POWER_PIN);
BME680Manager bme680;
INMP441Manager mic;
BatteryMonitor battery;

void setup() {
    Serial.begin(115200);
    Serial.print("Nombre d'endormissements: ");
    Serial.println(wake_count++);
   PowerOFF(POWER_PIN);
    delay(1000);
    
    // Phase 1: Mesure du niveau sonore pendant les premiers cycles
    if (wake_count < SOUND_MEASUREMENT_CYCLES) {
        measureSound();
        // Préparation pour le sommeil
        VextOFF();
        Radio.Sleep();
        SPI.end();
        gpio_hold_en((gpio_num_t)POWER_PIN);
        gpio_deep_sleep_hold_en();
        goToSleep(TIME_TO_SLEEP);
        return; // Sortie anticipée après la mesure de son
    }
    
    // Phase 2: Collecte et transmission des données complètes
    
    // Initialisation et mesure des particules fines
    // Réactivation de la broche Enable su TPS6203
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)POWER_PIN);
    
    particleSensor.begin();
    Serial.println("Capteur SDS011 initialisé");
    
    float pm25, pm10;
    if (!particleSensor.measure(pm25, pm10)) {
        Serial.println("{\"error\":\"Échec de la mesure de particules\"}");
    }
    
    // Initialisation et mesure BME680
    float sensorValues[8] = {0}; 
    if (!bme680.begin()) {
        Serial.println("Échec de l'initialisation du capteur BME680");
    } else {
        if (bme680.readData()) {
            bme680.getDataValues(sensorValues, true);
        }
        bme680.sleep();
    }
    
    // Mesure de la batterie
    battery.begin();
    battery.setMaxVoltage(3.9);
    battery.setMinVoltage(3.3);
    battery.setDividerRatio(2.745);
    battery.update();
    float batteryVoltage = battery.getVoltage();
    int batteryPercent = battery.getPercentage();
    
    // Transmission des données via LoRa
    initLoRa();
    
    char txpacket[BUFFER_SIZE];
    sprintf(txpacket, "%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%0.1f,%d,%d,%0.2f",
            pm25, pm10, sensorValues[0], sensorValues[1], sensorValues[2], sensorValues[4],
            sensorValues[6], sensorValues[7], dBMoyen, dBmax, dBmaxCount, batteryPercent,
            batteryVoltage);
    
    Serial.printf("\r\nEnvoi du paquet \"%s\" , longueur %d\r\n", txpacket, strlen(txpacket));
    sendLoRaPacket(txpacket);
    
    // Réinitialisation pour un nouveau cycle complet
    wake_count = 0;
    dBMoyen = 0;
    dBmax = 0;
    dBmaxCount = 0;
    prepareDeepSleep(POWER_PIN);
    goToSleep(TIME_TO_SLEEP);
}

void loop() {
    // Rien à faire dans la boucle, tout est géré dans setup() avec des cycles de veille
}

// Fonction pour mesurer le son
void measureSound() {
  float db;
    if (!mic.begin()) {
        Serial.println("Échec de l'initialisation du microphone INMP441");
        return;
    }
    
    Serial.println("INMP441 initialisé");
    //boucle pour stabiliser la mesure
    for (int i = 0; i< 10; i++){
      if (mic.readSound()) {
      db = mic.getDbValue();
      }
    }   
        //Serial.print("Niveau sonore");
        Serial.println(db);
        dBMoyen += db;
        dBMoyen /= 2;
        //Serial.print("Niveau sonore moyen");
        Serial.println(dBMoyen);
         
    }
