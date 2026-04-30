# Passerelle LoRa/WiFi – Heltec Wireless Stick Lite

## Vue d'ensemble

Ce projet transforme un **Heltec Wireless Stick Lite** (ESP32 + SX1262) en
passerelle entre un réseau de capteurs LoRa et la plateforme IoT
**ThingsBoard**, accessible via WiFi.

Au premier démarrage (ou après effacement de la configuration), le contrôleur
ouvre un portail web captif permettant de saisir les paramètres réseau. Une
fois configuré et redémarré, il passe en mode opérationnel : il écoute en
continu les paquets LoRa des capteurs et les retransmet vers ThingsBoard par
requête HTTP.

---

## Architecture des fichiers

```
heltec_gateway/
├── heltec_gateway.ino    # Point d'entrée : setup() / loop() + gestion bouton reset
├── config_portal.h       # Interface du portail captif
├── config_portal.cpp     # Implémentation : AP, DNS captif, serveur web, polling /status
├── lora_gateway.h        # Interface du mode opérationnel LoRa
├── lora_gateway.cpp      # Implémentation : init radio, réception, envoi HTTP
├── eeprom_utils.h        # Interface EEPROM
├── eeprom_utils.cpp      # Implémentation : lecture, écriture, effacement
└── README.md             # Cette documentation
```

---

## Logique de démarrage

```
Mise sous tension
      │
      ▼
Lecture EEPROM
(SSID, password, token)
      │
      ├─ Données présentes ──► Connexion WiFi (20 s max) ◄── bouton reset actif ici
      │                              │
      │                    ┌─────────┴──────────┐
      │                    │ Succès              │ Échec
      │                    ▼                    ▼
      │              Mode opérationnel    Portail captif
      │              (LoRa RX continu)    ◄── bouton reset actif ici
      │              ◄── bouton reset actif ici
      │
      └─ Données absentes ──► Portail captif
```

Le **bouton USR SW (GPIO0)** est surveillé à chaque étape, y compris pendant la
tentative de connexion WiFi. Un appui long de 3 secondes efface l'EEPROM et
relance le portail (voir section dédiée).

---

## Mode portail captif — configuration initiale

### Accès

1. Connectez votre smartphone ou PC au réseau Wi-Fi **`Heltec-Config`**
   (mot de passe : `12345678`).
2. Ouvrez un navigateur : toute URL redirige automatiquement vers la page de
   configuration grâce au DNS captif.

### Fonctionnalités de la page web

| Fonctionnalité | Description |
|---|---|
| Scan des réseaux disponibles | Liste automatique des SSID visibles, rafraîchissement toutes les 30 s |
| Sélection par clic | Un clic sur un réseau remplit le champ SSID instantanément |
| Afficher / masquer le mot de passe WiFi | Bouton 👁 à droite du champ, bascule entre `•••` et texte clair |
| Champ jeton capteur (Device Token) | Saisie du jeton d'authentification ThingsBoard |
| Afficher / masquer le jeton | Bouton 👁 identique sur le champ jeton |
| Confirmation de connexion en temps réel | La page affiche le statut de la tentative avant tout redémarrage |

### Procédure de saisie recommandée

1. Cliquer sur le réseau cible dans la liste — le champ SSID est rempli automatiquement.
2. Saisir le mot de passe WiFi, puis utiliser le bouton 👁 pour **vérifier
   visuellement** ce qui a été tapé avant de valider.
3. Saisir le jeton ThingsBoard, vérifier de même avec le bouton 👁.
4. Cliquer sur **Enregistrer & Connecter**.

### Mécanisme de confirmation de connexion avant redémarrage

L'un des problèmes classiques des portails captifs est de perdre la page web
au moment où le contrôleur bascule sur le réseau cible. Ce firmware contourne
ce problème de la façon suivante :

1. Le navigateur envoie `POST /save` avec SSID, mot de passe et jeton.
2. Le Heltec répond immédiatement `{"accepted":true}` **sans couper l'AP** :
   il passe en mode `WIFI_AP_STA` (AP et station simultanés).
3. La tentative de connexion au réseau cible est lancée en arrière-plan.
4. Le navigateur interroge l'endpoint `/status` toutes les 2 secondes
   (maximum 12 tentatives, soit ~24 secondes).
5. Selon le résultat :
   - **Succès** → `/status` retourne `{"state":"connected","ip":"…"}`. La page
     affiche l'adresse IP obtenue et annonce le redémarrage imminent. Les
     paramètres sont alors écrits en EEPROM. Le Heltec redémarre ~1,5 s plus tard.
   - **Échec** (délai dépassé ou mot de passe erroné) → `/status` retourne
     `{"state":"failed"}`. La page affiche un message d'erreur, le formulaire
     est réactivé, et **aucune donnée n'est écrite en EEPROM**. On peut
     corriger et réessayer sans redémarrer.

---

## ⚠️ Point d'attention — Commutation automatique de réseau WiFi

### Le problème

Les smartphones et certains PC portables implémentent une fonction de
**roaming automatique** (ou « connexion intelligente ») : si le signal du
réseau auquel ils sont connectés devient jugé insuffisant, l'appareil bascule
silencieusement vers un autre réseau connu offrant un meilleur signal — sans
avertissement et sans couper la session visible.

Pendant la saisie du mot de passe et du jeton sur le portail captif du Heltec,
cette commutation peut survenir à tout moment. Ses effets sont :

- La page web se fige ou affiche une erreur réseau.
- La saisie en cours est perdue.
- Le formulaire ne peut plus être soumis tant que l'appareil n'est pas
  reconnecté manuellement à `Heltec-Config`.

Ce problème est d'autant plus fréquent que le portail ne diffuse qu'en
**2,4 GHz à faible puissance**, alors que les réseaux domestiques ou de bureau
peuvent offrir un signal 5 GHz nettement plus fort à proximité.

### Solutions côté appareil (recommandé)

Avant d'ouvrir le portail de configuration, désactiver temporairement la
commutation automatique sur l'appareil utilisé :

**Android** : Paramètres → Wi-Fi → (nom du réseau `Heltec-Config`) → désactiver
« Passer automatiquement au réseau mobile » ou « Wi-Fi intelligent ».

**iOS / iPadOS** : Paramètres → Wi-Fi → (icône ⓘ à côté de `Heltec-Config`)
→ désactiver « Connexion automatique » et « Rejoindre automatiquement ».

**Windows 11** : Paramètres → Réseau et Internet → Wi-Fi → Gérer les réseaux
connus → supprimer les réseaux concurrents le temps de la configuration, ou
activer le mode avion sur les adaptateurs 5 GHz.

**macOS** : Préférences Système → Wi-Fi → décocher « Rejoindre
automatiquement » sur les autres réseaux présents dans la liste.

### Mesures implémentées dans le firmware

Du côté du Heltec, deux précautions sont prises pour limiter les effets de ce
problème :

- **`WiFi.setAutoReconnect(false)`** est appelé systématiquement avant toute
  tentative de connexion. Cela empêche l'ESP32 lui-même de basculer vers un
  autre réseau connu en mémoire.
- Le mode **`WIFI_AP_STA`** maintient l'AP actif pendant la tentative de
  connexion au réseau cible : si la commutation s'est produite et que
  l'utilisateur se reconnecte manuellement à `Heltec-Config`, la page `/status`
  reprend son polling et affiche le résultat final sans qu'il soit nécessaire
  de tout recommencer.

---

## Mode opérationnel — réception LoRa et envoi HTTP

### Paramètres radio (modifiables dans `lora_gateway.h`)

| Paramètre | Valeur par défaut |
|---|---|
| Fréquence | 868 MHz (plan de fréquences EU868) |
| Spreading Factor | SF7 |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Puissance TX | 20 dBm |
| Longueur de préambule | 8 symboles |
| CRC | activé |
| Réception continue | activée (`rxContinuous = true`) |

### Format du paquet LoRa

```
<DeviceToken>,<val1>,<val2>,…,<valN>
```

Le premier champ est le jeton du capteur. Il est comparé (`strcmp`) au jeton
stocké en EEPROM : tout paquet portant un jeton inconnu est ignoré
silencieusement. Les valeurs suivantes sont mappées dans l'ordre sur les clés
de télémétrie :

| Position | Clé JSON ThingsBoard |
|---|---|
| 1 | `pm25` |
| 2 | `pm10` |
| 3 | `temperature` |
| 4 | `humidite` |
| 5 | `pression` |
| 6 | `IAQ` |
| 7 | `eqCO2` |
| 8 | `eqCOV` |
| 9 | `dBSPL` |
| 10 | `BAT%` |
| 11 | `BATVolt` |

### Envoi HTTP vers ThingsBoard

```
POST http://aqor.fr:8080/api/v1/<DeviceToken>/telemetry
Content-Type: application/json

{"pm25":12.3 ,"pm10":18.0 ,"temperature":21.5 , …}
```

### Note sur la boucle de réception

`Radio.IrqProcess()` est appelé **sans aucun `delay()`** dans `loraLoop()`.
Le SDK Heltec traite les interruptions radio via un flag positionné par l'ISR
SPI et consulté par `IrqProcess()` : tout délai bloquant supérieur à quelques
millisecondes ferait manquer la fenêtre de déclenchement de la callback
`onRxDone`. Un heartbeat non-bloquant (point affiché toutes les 5 s via
`millis()`) permet de confirmer visuellement que la boucle tourne.

---

## Remise à zéro par le bouton USR SW (GPIO0)

### Objectif

Effacer la configuration stockée en EEPROM (SSID, mot de passe, jeton) sans
câble série ni reflashage du firmware. Cas d'usage typique : le réseau WiFi
cible a changé de nom ou de mot de passe et le contrôleur ne parvient plus à
s'y connecter.

### Procédure

1. **Maintenir appuyé** le bouton **USR SW** (marqué `USR` sur le PCB, GPIO0)
   pendant **plus de 3 secondes**.
2. Le moniteur série affiche un compte progressif (`0 s / 3 s`, `1 s / 3 s`,
   `2 s / 3 s`).
3. À 3 s : l'EEPROM est effacée, le contrôleur redémarre et ouvre
   automatiquement le portail de configuration.

Un appui bref (< 3 s) est ignoré pour éviter les effacements accidentels.

### Disponibilité du bouton

| Moment | Disponibilité |
|---|---|
| Pendant la tentative de connexion WiFi au démarrage | ✔ Vérifié à chaque demi-seconde |
| En mode portail captif | ✔ Vérifié à chaque itération de `loop()` |
| En mode opérationnel LoRa | ✔ Vérifié à chaque itération de `loop()` |

Le bouton est donc utilisable **à tout moment**, y compris pendant les 20
secondes d'attente de connexion au réseau enregistré — ce qui est le scénario
le plus courant en cas de changement de réseau.

### Détail technique

GPIO0 est configuré en entrée avec pull-up interne (`INPUT_PULLUP`). Le bouton
physique est câblé entre GPIO0 et GND : l'état actif est `LOW`. La fonction
`checkResetButton()` dans `heltec_gateway.ino` gère la détection et le
chronomètre de manière bloquante pendant la durée de l'appui (3 s au maximum),
puis reprend le cours normal si l'appui est relâché avant l'échéance.

---

## Persistance EEPROM

| Paramètre | Adresse de base | Longueur max |
|---|---|---|
| SSID | 0 | 99 caractères |
| Mot de passe WiFi | 100 | 99 caractères |
| Jeton capteur (Device Token) | 200 | 99 caractères |

Chaque champ est précédé d'un octet de longueur. La taille totale réservée est
**512 octets**. L'effacement via le bouton reset remet uniquement les octets de
longueur à zéro (adresses 0, 100 et 200), ce qui est suffisant pour que les
fonctions de lecture retournent une chaîne vide et déclenchent le portail au
prochain démarrage.

---

## Dépendances (bibliothèques Arduino)

| Bibliothèque | Rôle |
|---|---|
| `WiFi.h` | Connectivité WiFi ESP32 (modes STA, AP, AP+STA) |
| `WebServer.h` | Serveur HTTP embarqué du portail captif |
| `DNSServer.h` | DNS captif — redirige toute requête vers le portail |
| `EEPROM.h` | Stockage persistant des paramètres de configuration |
| `HTTPClient.h` | Envoi des télémétries vers ThingsBoard |
| `LoRaWan_APP.h` | Couche radio SX1262 — SDK officiel Heltec |

---

## Compilation

| Paramètre Arduino IDE | Valeur |
|---|---|
| Carte | `Heltec Wireless Stick Lite (V3)` |
| Package | Heltec ESP32 Series (gestionnaire de cartes Heltec) |
| Fréquence CPU | 240 MHz |
| Partition | Default |
| Port série (debug) | 115200 baud |

---

## Évolutions possibles

- Affichage de l'état (IP, RSSI LoRa, dernière trame reçue) sur l'écran OLED
  du Heltec, pour un diagnostic sans moniteur série.
- Mise à jour du firmware en OTA (Over-The-Air) via le portail captif, afin
  d'éviter tout branchement physique sur le terrain.
- Passage en HTTPS pour l'envoi des télémétries (ajout d'un certificat racine
  ThingsBoard dans le firmware).
- Gestion multi-capteurs : table de correspondance jeton → endpoint HTTP
  distinct par capteur.
- Sauvegarde horodatée des trames reçues en cas de perte de connexion WiFi
  (buffer circulaire en RAM ou sur flash SPI).
