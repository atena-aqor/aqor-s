// BatteryMonitor.h
#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>

class BatteryMonitor {
private:
  uint8_t _pinBat;              // Pin de l'ADC pour la batterie
  uint8_t _pinBatCtrl;          // Pin de contrôle pour activer la lecture de la batterie
  float _vRef;                  // Tension de référence (3.3V par défaut)
  float _dividerRatio;          // Rapport du diviseur de tension
  uint8_t _numSamples;          // Nombre d'échantillons pour le lissage
  
  // Seuils de tension pour calculer le pourcentage
  float _maxVoltage;            // 100% de la batterie
  float _minVoltage;            // 0% de la batterie
  
  // Résultats
  float _voltage;               // Tension calculée
  int _percentage;              // Pourcentage calculé

public:
  // Constructeur avec paramètres par défaut
  BatteryMonitor(
    uint8_t pinBat = 1,        // Pin par défaut sur Heltec
    uint8_t pinBatCtrl = 37,     // Pin de contrôle de la batterie (VBAT_READ_CNTRL_PIN)
    float vRef = 3.3,           // Tension de référence 3.3V
    float dividerRatio = 2.74,   // Diviseur de tension par 2
    uint8_t numSamples = 10,    // 10 échantillons par défaut
    float maxVoltage = 4.2,     // Tension max LiPo
    float minVoltage = 3.3      // Tension min LiPo
  );
  
  // Initialisation de l'ADC et du pin de contrôle
  void begin();
  
  // Mesure de la batterie
  void update();
  
  // Getters
  float getVoltage() const { return _voltage; }
  int getPercentage() const { return _percentage; }
  
  // Calibration
  void setMaxVoltage(float voltage) { _maxVoltage = voltage; }
  void setMinVoltage(float voltage) { _minVoltage = voltage; }
  void setDividerRatio(float ratio) { _dividerRatio = ratio; }
};

#endif
