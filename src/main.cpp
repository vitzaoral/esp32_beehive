#include <Arduino.h>
#include <InternetConnection.h>
#include <Ticker.h>

InternetConnection connection;
MeteoData meteoData;
PowerController powerController;
GyroscopeController gyroscopeController;

void sendDataToInternet();
void checkIncomingCall();
void checkGyroscopeAlarm();
Ticker timerSendDataToInternet(sendDataToInternet, 300000); // 5 min
Ticker timerCheckIncomingCall(checkIncomingCall, 2000);     // 2 sec
Ticker timerGyroscopeAlarm(checkGyroscopeAlarm, 2500);      // 2.5 sec

void setup()
{
  Serial.begin(115200);
  connection.initialize();

  timerSendDataToInternet.start();
  timerCheckIncomingCall.start();
  timerGyroscopeAlarm.start();

  // set first gyroscope data for first send, other in timer
  gyroscopeController.setData();
  Serial.println("Setup done, send first data");
  sendDataToInternet();
  Serial.println("First data sended, start loop");
}

void loop()
{
  timerSendDataToInternet.update();
  timerCheckIncomingCall.update();
  timerGyroscopeAlarm.update();
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
    // gyroscope data are set in other timer

    Serial.println("Sending data to Blynk");
    connection.sendDataToBlynk(meteoData, powerController, gyroscopeController);
    connection.disconnect();
  }
  else
  {
    Serial.println("No internet connection");
  }
}

void checkGyroscopeAlarm()
{
  gyroscopeController.setData();
  // TODO: alarm...
}