/*
 * config.h - Fichier de configuration pour EmetteurAQOR
 * 
 * Ce fichier contient toutes les constantes et définitions
 * globales utilisées par le programme.
 */

#ifndef CONFIG_H
#define CONFIG_H

// Constantes de temporisation
#define TIME_TO_SLEEP 5            // Durée de sommeil de l'ESP32 (secondes)
#define TIME_TO_MEASURE_PM 20      // Durée entre deux mesures PM (secondes)
#define SOUND_MEASUREMENT_CYCLES 40 // Nombre de cycles pour mesurer le son

// Définition des broches pour le capteur SDS011
#define RX_PIN 5                   // GPIO5 pour RXD
#define TX_PIN 4                   // GPIO4 pour TXD
#define POWER_PIN 21               // GPIO21 pour alimenter le capteur

// Configuration LoRa
#define RF_FREQUENCY 868000000     // Hz
#define TX_OUTPUT_POWER 18         // dBm
#define LORA_BANDWIDTH 0           // 0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved
#define LORA_SPREADING_FACTOR 7    // SF7..SF12
#define LORA_CODINGRATE 1          // 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8
#define LORA_PREAMBLE_LENGTH 8     // Même pour Tx et Rx
#define LORA_SYMBOL_TIMEOUT 0      // Symboles
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 128            // Taille du payload

// Autres définitions
#define Vext 43                    // Broche d'alimentation externe (peut varier selon la carte)

#endif // CONFIG_H
