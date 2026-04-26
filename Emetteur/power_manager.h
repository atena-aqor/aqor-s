/*
 * power_manager.h - Gestionnaire d'alimentation et de sommeil
 * 
 * Ce fichier contient les fonctions liées à la gestion de l'alimentation
 * et du sommeil profond de l'ESP32.
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "Arduino.h"
#include "config.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "driver/gpio.h"

// Activation de l'alimentation externe
void VextON() {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);  // LOW active l'alimentation
}

// Désactivation de l'alimentation externe
void VextOFF() {
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, HIGH); // HIGH désactive l'alimentation
}

// Désactivation de l'alimentation du SDS011
void PowerOFF(int powerPin){
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin,LOW);
}

// Préparation au sommeil profond
void prepareDeepSleep(int powerPin) {
    VextOFF();
    PowerOFF(powerPin);
    Radio.Sleep();
    SPI.end();
    
    // Conservation de l'état des broches pendant le sommeil
    gpio_hold_en((gpio_num_t)powerPin);
    gpio_deep_sleep_hold_en();
}

// Mise en sommeil profond
void goToSleep(int sleepSeconds) {
    Radio.Sleep();
    delay(100);
    VextOFF();
    
    // Configuration des pins pour le mode sommeil
    // Heltec V3 configuration
    pinMode(37, INPUT);      // ADC_CTRL 
    pinMode(9, INPUT);       // LORA SCK
    pinMode(14, INPUT_PULLUP); // LORA RST
    pinMode(8, INPUT_PULLUP);  // LORA NSS
    pinMode(11, INPUT);      // LORA MISO
    pinMode(12, INPUT);      // DIO1 (LORA)
    pinMode(10, INPUT);      // LORA MOSI
    pinMode(41, INPUT);      // I2C_SDA
    pinMode(42, INPUT);      // I2C_SCL
    pinMode(39, INPUT);      // I2S_SCK Serial Clock
    pinMode(38, INPUT);      // I2S_WS Word Select
    pinMode(40, INPUT);      // I2S_SD Serial Data
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    
    Serial.printf("Deep sleeping for %d seconds\n", sleepSeconds);
    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000);
    Serial.flush();
    esp_deep_sleep_start();
}

#endif // POWER_MANAGER_H
