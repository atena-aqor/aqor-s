#ifndef ESP32S3_CRYPTER_H
#define ESP32S3_CRYPTER_H
/*#ifndef ESP32S3Crypter
#define ESP32S3Crypter*/

#include <Arduino.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <mbedtls/aes.h>
#include <mbedtls/md.h>

class ESP32S3Crypter {
private:
    uint8_t key[16];
    void generateKeyFromMAC();

public:
    ESP32S3Crypter();
    
    void encrypt(const uint8_t* input, size_t input_len, 
                 uint8_t* output, size_t* output_len);
    
    void decrypt(const uint8_t* input, size_t input_len, 
                 uint8_t* output, size_t* output_len);
};

#endif // ESP32S3Crypter
