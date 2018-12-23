#ifndef __MeteoData_H
#define __MeteoData_H

#include <Arduino.h>
#include <DHTesp.h>

// struct TempAndHumidity
// {
//     float temperature;
//     float humidity;
// };

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