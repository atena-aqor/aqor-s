#include "ESP32S3Crypter.h"

void ESP32S3Crypter::generateKeyFromMAC() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    
    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);
    mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
    mbedtls_md_starts(&md_ctx);
    mbedtls_md_update(&md_ctx, mac, 6);
    uint8_t sha_result[32];
    mbedtls_md_finish(&md_ctx, sha_result);
    
    memcpy(key, sha_result, 16);
    mbedtls_md_free(&md_ctx);
}

ESP32S3Crypter::ESP32S3Crypter() {
    generateKeyFromMAC();
}

void ESP32S3Crypter::encrypt(const uint8_t* input, size_t input_len, 
                              uint8_t* output, size_t* output_len) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    uint8_t iv[16];
    esp_fill_random(iv, 16);
    
    mbedtls_aes_setkey_enc(&aes, key, 128);
    
    size_t padded_len = ((input_len + 15) / 16) * 16;
    uint8_t* padded_input = new uint8_t[padded_len];
    memcpy(padded_input, input, input_len);
    memset(padded_input + input_len, padded_len - input_len, padded_len - input_len);
    
    memcpy(output, iv, 16);
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, padded_len, iv, padded_input, output + 16);
    
    *output_len = padded_len + 16;
    
    delete[] padded_input;
    mbedtls_aes_free(&aes);
}

void ESP32S3Crypter::decrypt(const uint8_t* input, size_t input_len, 
                              uint8_t* output, size_t* output_len) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    
    uint8_t iv[16];
    memcpy(iv, input, 16);
    
    mbedtls_aes_setkey_dec(&aes, key, 128);
    
    size_t ciphertext_len = input_len - 16;
    mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, ciphertext_len, iv, input + 16, output);
    
    uint8_t padding_length = output[ciphertext_len - 1];
    *output_len = ciphertext_len - padding_length;
    
    mbedtls_aes_free(&aes);
}
