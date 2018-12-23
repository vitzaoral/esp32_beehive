#include <Arduino.h>
#include <InternetConnection.h>
#include <Ticker.h>

InternetConnection connection;
MeteoData meteoData;
PowerController powerController;

void sendDataToInternet();
void checkIncomingCall();
Ticker timerSendDataToInternet(sendDataToInternet, 300000);
Ticker timerCheckIncomingCall(checkIncomingCall, 2000);

void setup()
{
  Serial.begin(115200);
  connection.initialize();

  timerSendDataToInternet.start();
  timerCheckIncomingCall.start();

  Serial.println("Setup done");
}

void loop()
{
  timerSendDataToInternet.update();
  timerCheckIncomingCall.update();
}

void checkIncomingCall()
{
  connection.checkIncomingCall();
}

void sendDataToInternet()
{
  Serial.println("Start initialize Blynk connection");
  if (connection.initializeConnection())
  {
    Serial.println("Setting sensors data");
    meteoData.setData();
    powerController.setData();

    Serial.println("Sending data to Blynk");
    connection.sendDataToBlynk(meteoData, powerController);

    connection.disconnect();
  }
  else
  {
    Serial.println("No internet connection");
  }
}