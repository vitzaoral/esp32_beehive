#ifndef __InternetConnection_H
#define __InternetConnection_H

// SIM800L chip
#define TINY_GSM_MODEM_SIM800

// microphones turn off/on pins
#define MICROPHONE_A_PIN 23
#define MICROPHONE_B_PIN 32
#define MICROPHONE_C_PIN 33

#include <TinyGsmClient.h>
#include <MeteoData.h>
#include <PowerController.h>
#include <GyroscopeController.h>
#include <MagneticLockController.h>
#include <SirenController.h>

class InternetConnection
{
public:
  void initialize();
  bool modemReady;
  bool isAlarm;
  void checkIncomingCall();
  bool initializeConnection();
  void disconnect();
  void sendDataToBlynk(MeteoData, PowerController, GyroscopeController, MagneticLockController);
  void alarmMagneticController(MagneticLockController);
  void blynkRunIfAlarm();
  void setMagneticLockControllerDataToBlynkIfAlarm(MagneticLockController);

private:
  void restartModem();
  void setMagneticLockControllerDataToBlynk(MagneticLockController);
  void getSignalQualityDescription(int, int);
  void processIncomingCall();
  void setAlarmCollor(int, bool);
};

#endif