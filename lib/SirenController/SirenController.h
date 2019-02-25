#ifndef __SirenController_H
#define __SirenController_H

#include <Arduino.h>

#define BUZZER_PIN 4

class SirenController
{
  public:
    SirenController();
    void sirenOn();
    void sirenOff();
};

#endif