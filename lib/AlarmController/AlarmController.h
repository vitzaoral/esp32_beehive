#ifndef __AlarmController_H
#define __AlarmController_H

#include <InternetConnection.h>

#define BUZZER_PIN 15

class AlarmController
{
  public:
    AlarmController();
    void alarmOn();
    void alarmOff();
};

#endif