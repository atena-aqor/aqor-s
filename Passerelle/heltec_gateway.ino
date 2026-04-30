/**
 * @file heltec_gateway.ino
 * @brief Passerelle LoRa/WiFi pour réseau de capteurs - Point d'entrée principal
 *
 * Ce fichier orchestre le démarrage :
 *  - Si des identifiants WiFi + jeton sont déjà stockés en EEPROM, on tente
 *    la connexion au réseau cible puis on passe en mode opérationnel LoRa.
 *  - Sinon (ou si la connexion échoue), on lance le portail captif de
 *    configuration WiFi (mode AP).
 *
 * Remise à zéro de l'EEPROM :
 *  - Appui long (> 3 s) sur le bouton USR SW (GPIO0) à n'importe quel moment
 *    → efface l'EEPROM et redémarre vers le portail de configuration.
 *
 * Architecture :
 *   heltec_gateway.ino   – setup() / loop() principal + gestion bouton
 *   config_portal.h/.cpp – portail captif (AP + serveur web + DNS)
 *   lora_gateway.h/.cpp  – réception LoRa et envoi HTTP vers ThingsBoard
 *   eeprom_utils.h/.cpp  – lecture / écriture / effacement EEPROM
 */

#include "config_portal.h"
#include "lora_gateway.h"
#include "eeprom_utils.h"

// ── Paramètres du point d'accès de configuration ────────────────────────────
const char* AP_SSID     = "Heltec-Config";
const char* AP_PASSWORD = "12345678";

// ── Bouton USR SW ────────────────────────────────────────────────────────────
constexpr uint8_t  BTN_PIN          = 0;     // GPIO0 – actif à l'état BAS
constexpr uint32_t BTN_LONG_PRESS_MS = 3000; // durée d'appui pour effacement

// ── Variables d'état global ──────────────────────────────────────────────────
bool configMode = false;   ///< true = portail actif, false = mode opérationnel

// ============================================================================
//  Gestion du bouton USR SW (appui long = reset EEPROM)
//
//  À appeler régulièrement : dans la boucle de connexion WiFi ET dans loop().
//  Retourne true si un appui long a été détecté (le reboot est alors lancé
//  directement depuis cette fonction).
// ============================================================================
static void checkResetButton() {
    // Le bouton est actif bas (pull-up interne)
    if (digitalRead(BTN_PIN) != LOW) return;   // pas appuyé

    // Appui détecté : on chronomètre
    uint32_t pressStart = millis();
    Serial.println("[BTN] Appui détecté – maintenir 3 s pour effacer l'EEPROM…");

    while (digitalRead(BTN_PIN) == LOW) {
        uint32_t held = millis() - pressStart;

        // Feedback visuel dans le moniteur série toutes les 500 ms
        if (held % 500 < 20) {
            Serial.printf("[BTN] %lu s / 3 s\n", held / 1000);
        }

        if (held >= BTN_LONG_PRESS_MS) {
            Serial.println("[BTN] *** Effacement EEPROM et redémarrage ***");
            eepromClear();
            delay(200);
            ESP.restart();
            // — jamais atteint —
        }
        delay(20);
    }

    Serial.println("[BTN] Appui trop court – ignoré.");
}

// ============================================================================
void setup() {
    Serial.begin(115200);
    Mcu.begin();

    // Configuration du bouton USR SW avec pull-up interne
    pinMode(BTN_PIN, INPUT_PULLUP);

    // Initialisation EEPROM et lecture des valeurs stockées
    eepromInit();
    String storedSSID     = eepromReadSSID();
    String storedPassword = eepromReadPassword();
    String storedToken    = eepromReadToken();

    Serial.println("=== Heltec LoRa/WiFi Gateway ===");
    Serial.println("SSID stocké   : " + storedSSID);
    Serial.println("Jeton stocké  : " + storedToken);
    Serial.println("(Appui long sur USR SW pour effacer la config)");

    // ── Tentative de connexion au réseau enregistré ──────────────────────────
    if (storedSSID.length() > 0 && storedPassword.length() > 0) {
        Serial.println("Connexion au réseau : " + storedSSID);

        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(false);
        WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

        // Boucle de connexion : on vérifie le bouton à chaque itération
        // ce qui permet de déclencher le reset même pendant cette attente.
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 40) {
            checkResetButton();   // ← interception bouton pendant la connexion
            delay(500);
            Serial.print(".");
            attempts++;
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi connecté ! IP : " + WiFi.localIP().toString());
            delay(300);   // stabilisation SPI avant init radio
            loraInit(storedToken);
            configMode = false;
            return;
        } else {
            Serial.println("Échec de la connexion WiFi – lancement du portail.");
        }
    } else {
        Serial.println("Aucune configuration stockée – lancement du portail.");
    }

    // ── Mode portail captif ──────────────────────────────────────────────────
    portalStart(AP_SSID, AP_PASSWORD);
    configMode = true;
}

// ============================================================================
void loop() {
    // Le bouton est vérifié en premier dans tous les modes
    checkResetButton();

    if (configMode) {
        portalLoop();

        if (portalShouldReboot()) {
            Serial.println("Redémarrage suite à la configuration...");
            delay(1500);
            ESP.restart();
        }
    } else {
        loraLoop();
    }
}

