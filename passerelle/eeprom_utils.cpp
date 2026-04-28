/**
 * @file eeprom_utils.cpp
 * @brief Implémentation des fonctions EEPROM
 */

#include "eeprom_utils.h"

// ── Helpers internes ─────────────────────────────────────────────────────────

static void writeString(int address, const String& data) {
    uint8_t len = (uint8_t)min((int)data.length(), 99);
    EEPROM.write(address, len);
    for (int i = 0; i < len; i++) {
        EEPROM.write(address + 1 + i, (uint8_t)data[i]);
    }
}

static String readString(int address) {
    uint8_t len = EEPROM.read(address);
    if (len > 99) return "";   // valeur non initialisée (0xFF)
    String result = "";
    result.reserve(len);
    for (int i = 0; i < len; i++) {
        result += (char)EEPROM.read(address + 1 + i);
    }
    return result;
}

// ── API publique ─────────────────────────────────────────────────────────────

void eepromInit() {
    EEPROM.begin(EEPROM_TOTAL_SIZE);
}

String eepromReadSSID() {
    return readString(EEPROM_SSID_ADDR);
}

String eepromReadPassword() {
    return readString(EEPROM_PWD_ADDR);
}

String eepromReadToken() {
    return readString(EEPROM_TOKEN_ADDR);
}

void eepromSaveAll(const String& ssid, const String& password, const String& token) {
    writeString(EEPROM_SSID_ADDR,  ssid);
    writeString(EEPROM_PWD_ADDR,   password);
    writeString(EEPROM_TOKEN_ADDR, token);
    EEPROM.commit();
    Serial.println("[EEPROM] Sauvegarde : SSID=" + ssid + "  Token=" + token);
}

void eepromClear() {
    // On efface uniquement les octets de longueur de chaque champ.
    // Mettre len=0 suffit : readString() retournera "" pour chaque paramètre.
    EEPROM.write(EEPROM_SSID_ADDR,  0);
    EEPROM.write(EEPROM_PWD_ADDR,   0);
    EEPROM.write(EEPROM_TOKEN_ADDR, 0);
    EEPROM.commit();
    Serial.println("[EEPROM] Effacement effectué.");
}
