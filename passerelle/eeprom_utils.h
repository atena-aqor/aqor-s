/**
 * @file eeprom_utils.h
 * @brief Lecture / écriture des paramètres persistants en EEPROM
 *
 * Disposition mémoire :
 *   Adresse   0 : SSID     (longueur 1 octet + données, max 99 caractères)
 *   Adresse 100 : Password (longueur 1 octet + données, max 99 caractères)
 *   Adresse 200 : Token    (longueur 1 octet + données, max 99 caractères)
 */

#pragma once
#include <Arduino.h>
#include <EEPROM.h>

// ── Adresses EEPROM ──────────────────────────────────────────────────────────
constexpr int EEPROM_TOTAL_SIZE  = 512;
constexpr int EEPROM_SSID_ADDR   =   0;
constexpr int EEPROM_PWD_ADDR    = 100;
constexpr int EEPROM_TOKEN_ADDR  = 200;

// ── Prototypes ───────────────────────────────────────────────────────────────

/** Initialise la bibliothèque EEPROM – à appeler une fois dans setup(). */
void eepromInit();

/** Lit et retourne le SSID stocké. */
String eepromReadSSID();

/** Lit et retourne le mot de passe stocké. */
String eepromReadPassword();

/** Lit et retourne le jeton du capteur stocké. */
String eepromReadToken();

/**
 * Sauvegarde les trois paramètres d'un coup et effectue un commit.
 * @param ssid     Nom du réseau WiFi cible
 * @param password Mot de passe du réseau WiFi cible
 * @param token    Jeton d'authentification du capteur (ThingsBoard)
 */
void eepromSaveAll(const String& ssid, const String& password, const String& token);

/**
 * Efface tous les paramètres stockés (écrit 0 sur les octets de longueur).
 * Appelle EEPROM.commit() automatiquement.
 */
void eepromClear();
