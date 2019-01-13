#ifndef __InternetConnection_H
#define __InternetConnection_H

// SIM800L chip
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <MeteoData.h>
#include <PowerController.h>
#include <GyroscopeController.h>
#include <MagneticLockController.h>

class InternetConnection
{
public:
  void initialize();
  bool modemReady;
  void checkIncomingCall();
  bool initializeConnection();
  void disconnect();
  void sendDataToBlynk(MeteoData, PowerController, GyroscopeController, MagneticLockController);

private:
  void restartModem();
  void getSignalQualityDescription(int, int);
};

#endif