#ifndef __InternetConnection_H
#define __InternetConnection_H

// SIM800L chip
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>
#include <MeteoData.h>
#include <PowerController.h>

class InternetConnection
{
public:
  InternetConnection();
  bool modemReady;
  void checkIncomingCall();
  bool initializeConnection();
  void disconnect();
  void sendDataToBlynk(MeteoData, PowerController);
};

#endif