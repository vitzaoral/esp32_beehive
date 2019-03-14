#ifndef __InternetConnection_H
#define __InternetConnection_H

// SIM800L chip
#define TINY_GSM_MODEM_SIM800

// microphones turn off/on pins
#define MICROPHONE_A_PIN 23
#define MICROPHONE_B_PIN 32
#define MICROPHONE_C_PIN 33

// pin for modem hardware reset
#define MODEM_RESET_PIN 2

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Update.h>
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
  void alarmGyroscopeController(GyroscopeController);
  void blynkRunIfAlarm();
  void setMagneticLockControllerDataToBlynkIfAlarm(MagneticLockController);
  void setGyroscopeControllerDataToBlynkIfAlarm(GyroscopeController);
  void checkNewVersionAndUpdate();
  void printlnToTerminal(String);
  void checkSirenAlarm();

private:
  void restartModem();
  void setMagneticLockControllerDataToBlynk(MagneticLockController);
  void setGyroscopeControllerDataToBlynk(GyroscopeController);
  void setAlarmInfoToBlynk();
  void getSignalQualityDescription(int, int);
  void processIncomingCall();
  void setAlarmCollor(int, bool);
  void updateFirmware(HttpClient);
  void printPercent(uint32_t, uint32_t);
  void performUpdate(Stream &, size_t);
  void updateFromFS();
};

#endif