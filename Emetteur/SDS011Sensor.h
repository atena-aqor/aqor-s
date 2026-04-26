#ifndef SDS011_SENSOR_H
#define SDS011_SENSOR_H

#include <HardwareSerial.h>
#include <stdint.h>  // Pour uint8_t

class SDS011Sensor {
public:
    // Constructeur
    SDS011Sensor(int rxPin, int txPin, int powerPin);
    
    // Méthodes publiques
    void begin();
    void powerOn();
    void powerOff();
    bool readSensor(float& pm25, float& pm10);
    bool measure(float& pm25, float& pm10, int warmupTime = 10000, int sampleTime = 5000);
    
    // Méthode pour récupérer les données en JSON
    String getJSON(float pm25, float pm10);

private:
    HardwareSerial _sdsSerial;
    int _rxPin;
    int _txPin;
    int _powerPin;
    uint8_t _buffer[10];
};

#endif // SDS011_SENSOR_H
