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
  void initialize();
  bool modemReady;
  void checkIncomingCall();
  bool initializeConnection();
  void disconnect();
  void sendDataToBlynk(MeteoData, PowerController);

private:
  void restartModem();
};

#endif