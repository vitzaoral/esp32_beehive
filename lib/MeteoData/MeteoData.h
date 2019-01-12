#ifndef __MeteoData_H
#define __MeteoData_H

#include <Arduino.h>
#include <Adafruit_SHT31.h>

#define SENSOR_A_PIN 18
#define SENSOR_B_PIN 19
#define SENSOR_C_PIN 5

struct TempAndHumidity
{
    float temperature;
    float humidity;
};

class MeteoData
{
  public:
    MeteoData();
    TempAndHumidity sensorA;
    TempAndHumidity sensorB;

    void setData();
    bool dataAreValid(TempAndHumidity);
};

#endif
