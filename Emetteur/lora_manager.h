/*
 * lora_manager.h - Gestionnaire de communication LoRa
 * 
 * Ce fichier contient les fonctions liées à la gestion 
 * et la communication via le module LoRa.
 */

#ifndef LORA_MANAGER_H
#define LORA_MANAGER_H

#include "LoRaWan_APP.h"
#include "config.h"

// Variables globales pour LoRa
static RadioEvents_t RadioEvents;
bool lora_idle = true;
char rxpacket[BUFFER_SIZE];

// Fonctions de callback pour les événements LoRa
void OnTxDone(void) {
    Serial.println("TX done......");
    lora_idle = true;
}

void OnTxTimeout(void) {
    Radio.Sleep();
    Serial.println("TX Timeout......");
    lora_idle = true;
}

// Initialisation du module LoRa
void initLoRa() {
    Mcu.begin();
    
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    
    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(
        MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
        true, 0, 0, LORA_IQ_INVERSION_ON, 3000
    );
    
    lora_idle = true;
    Serial.println("Module LoRa initialisé");
}

// Envoi d'un paquet LoRa
bool sendLoRaPacket(const char* data) {
    if (!lora_idle) {
        Serial.println("LoRa occupé, impossible d'envoyer");
        return false;
    }
    
    lora_idle = false;
    Radio.Send((uint8_t*)data, strlen(data));
    
    // Attente de la fin de la transmission
    unsigned long startTime = millis();
    while (!lora_idle && (millis() - startTime < 5000)) {
        Radio.IrqProcess();
        delay(10);
    }
    
    if (!lora_idle) {
        Serial.println("Timeout lors de l'envoi du paquet");
        lora_idle = true;
        return false;
    }
    
    Serial.println("Paquet transmis avec succès");
    return true;
}

#endif // LORA_MANAGER_H
