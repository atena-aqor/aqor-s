#ifndef INMP441_MANAGER_H
#define INMP441_MANAGER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <ArduinoJson.h>
#include <math.h>

class INMP441Manager {
private:
    // Configuration des broches I2S
    const int i2s_ws_pin = 38;    // Word Select (WS)
    const int i2s_sck_pin = 39;   // Serial Clock (SCK)
    const int i2s_sd_pin = 40;    // Serial Data (SD)
    
    // Configuration I2S
    const i2s_port_t i2s_port = I2S_NUM_0;
    const int sample_rate = 44100;
    const int buffer_len = 1024;
    int32_t* i2s_buffer;
    
    // Variables pour le calcul du niveau sonore
    float db_value = 0.0;
    const float reference_pressure = 0.00002; // 20 μPa (référence pour dB SPL)
    
    // Calibration du microphone
    const float mic_sensitivity = -26.0; // dBFS @ 94dB SPL (valeur typique pour INMP441)
    const float mic_reference_level = 94.0; // dB SPL de référence pour la calibration
    const float adc_reference = 3.3; // Tension de référence de l'ADC en volts
    const int adc_bits = 24; // Bits de résolution de l'ADC
    
public:
    INMP441Manager() {
        i2s_buffer = new int32_t[buffer_len];
    }
    
    ~INMP441Manager() {
        if (i2s_buffer != nullptr) {
            delete[] i2s_buffer;
        }
    }
    
    bool begin() {
        // Configuration I2S
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = sample_rate,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 4,
            .dma_buf_len = buffer_len,
            .use_apll = false,
            .tx_desc_auto_clear = false,
            .fixed_mclk = 0
        };
        
        // Configuration des broches I2S
        i2s_pin_config_t pin_config = {
            .bck_io_num = i2s_sck_pin,
            .ws_io_num = i2s_ws_pin,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num = i2s_sd_pin
        };
        
        // Installer le pilote I2S
        esp_err_t result = i2s_driver_install(i2s_port, &i2s_config, 0, NULL);
        if (result != ESP_OK) {
            Serial.println("Erreur lors de l'installation du pilote I2S");
            return false;
        }
        
        // Configurer les broches I2S
        result = i2s_set_pin(i2s_port, &pin_config);
        if (result != ESP_OK) {
            Serial.println("Erreur lors de la configuration des broches I2S");
            return false;
        }
        
        Serial.println("Microphone INMP441 initialisé avec succès");
        return true;
    }
    
    void stop() {
        i2s_driver_uninstall(i2s_port);
        Serial.println("Microphone INMP441 arrêté");
    }
    
    bool readSound() {
        size_t bytes_read = 0;
        
        // Lire les données du microphone
        esp_err_t result = i2s_read(i2s_port, i2s_buffer, buffer_len * sizeof(int32_t), &bytes_read, portMAX_DELAY);
        if (result != ESP_OK) {
            Serial.println("Erreur lors de la lecture des données I2S");
            return false;
        }
        
        int samples_read = bytes_read / sizeof(int32_t);
        //Serial.print("samples number : ");
        //Serial.println(samples_read);
        if (samples_read == 0) {
            return false;
        }
        
        // Calculer la valeur RMS
        double sum_squares = 0.0;
        for (int i = 0; i < samples_read; i++) {
            // Convertir la valeur 32 bits signée en valeur normalisée entre -1.0 et 1.0
            float sample = (float)i2s_buffer[i] / (float)(1UL << 31);
            //Serial.println(1UL << 31);
            sum_squares += sample * sample;
        }
        
        float rms = sqrt(sum_squares / samples_read);
        
        // Convertir RMS en dB SPL (Sound Pressure Level)
        // Ajustement pour la sensibilité du microphone et le niveau de référence
        float db_fs = 20.0 * log10(rms); // dB Full Scale
        
        // Conversion en dB SPL avec calibration
        db_value = db_fs - mic_sensitivity + mic_reference_level;
        
        // Limiter les valeurs possibles entre 30 et 120 dB SPL (plage typique audible)
        if (db_value < 20.0) db_value = 20.0;
        if (db_value > 120.0) db_value = 120.0;
        
        return true;
    }
    
    String getJsonData() {
        // Créer un objet JSON avec les données du microphone
        StaticJsonDocument<128> jsonDoc;
        
        jsonDoc["db_spl"] = db_value;
        jsonDoc["timestamp"] = millis();
        
        String jsonString;
        serializeJson(jsonDoc, jsonString);
        
        return jsonString;
    }
    
    float getDbValue() {
        return db_value;
    }
};

#endif // INMP441_MANAGER_H
