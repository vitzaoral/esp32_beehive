#ifndef __PowerController_H
#define __PowerController_H

#include <Adafruit_INA219.h>

class PowerController
{
  public:
    PowerController();
    float busVoltage;
    float shuntVoltage;
    float loadVoltage;
    float current_mA;
    float power_mW;

    void setData();
};

#endif