// BatteryMonitor.cpp
#include "BatteryMonitor.h"


BatteryMonitor::BatteryMonitor(uint8_t pinBat, uint8_t pinBatCtrl, float vRef, float dividerRatio, uint8_t numSamples, float maxVoltage, float minVoltage) {
  _pinBat = pinBat;
  _pinBatCtrl = pinBatCtrl;
  _vRef = vRef;
  _dividerRatio = dividerRatio;
  _numSamples = numSamples;
  _maxVoltage = maxVoltage;
  _minVoltage = minVoltage;
  
  _voltage = 0.0;
  _percentage = 0;
}

void BatteryMonitor::begin() {
  // Configuration du pin de contrôle comme sortie
  pinMode(_pinBatCtrl, OUTPUT);
  digitalWrite(_pinBatCtrl, HIGH);  // Désactiver la lecture au démarrage
  
  // Initialisation de l'ADC pour l'ESP32
  adcAttachPin(_pinBat);
  analogSetPinAttenuation(_pinBat, ADC_6db); // Configuration pour plage 0-3.3V
  analogReadResolution(12); // Configuration de la résolution de l'ADC
}

void BatteryMonitor::update() {
  long sumReadings = 0;
  
  // Activer la lecture de la batterie
  digitalWrite(_pinBatCtrl, LOW);
  
  // Petit délai pour stabiliser la lecture
  delay(20);
  
  // Prendre plusieurs échantillons pour améliorer la précision
  for(int i = 0; i < _numSamples; i++) {
    sumReadings += analogRead(_pinBat);
    delay(5);
  }
  
  // Désactiver la lecture de la batterie pour économiser l'énergie
  digitalWrite(_pinBatCtrl, HIGH);
  
  float avgReading = sumReadings / (float)_numSamples;
  
  // Conversion de la valeur ADC en tension
  // La valeur max de l'ADC (4095) correspond à _vRef
  // On applique le diviseur de tension pour obtenir la tension réelle
  _voltage = (avgReading * _vRef * _dividerRatio) / 4095.0;
  
  // Calcul du pourcentage
  if(_voltage >= _maxVoltage) {
    _percentage = 100;
  } else if(_voltage <= _minVoltage) {
    _percentage = 0;
  } else {
    _percentage = int((_voltage - _minVoltage) / (_maxVoltage - _minVoltage) * 100.0);
  }
}
