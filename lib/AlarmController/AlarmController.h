#ifndef __AlarmController_H
#define __AlarmController_H

#include <Arduino.h>

#define BUZZER_PIN 4

class AlarmController
{
  public:
    AlarmController();
    void alarmOn();
    void alarmOff();
};

#endif