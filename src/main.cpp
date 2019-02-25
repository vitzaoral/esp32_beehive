#include <Arduino.h>
#include <InternetConnection.h>
#include <Ticker.h>

// cervenby S09 start 13.2. v 21:30 - 14.2. 23:30 - 26 hodin
// DD0503MA start 15.2. 7:30 - 16.2. 13:45 - 30 hodin
// 403MA start 18.2. 7:30 - 19.2. 11:45 - 28 hodin

// DD0503MA bez senzoru start 20.2. 7:30 - 15:30 - 32 hodin

// TODO: zkusit max. napeti pro TP4056 a na to nastavit regulator za solarem (ale bude to fungovat pro mensi napeti..?)

InternetConnection connection;
MeteoData meteoData;
PowerController powerController;
GyroscopeController gyroscopeController;
MagneticLockController magneticLockController;
SirenController SirenController;

void sendDataToInternet();
void checkIncomingCall();
void checkGyroscopeAlarm();
void checkMagneticLockAlarm();
Ticker timerSendDataToInternet(sendDataToInternet, 300000);  // 5 min
Ticker timerCheckIncomingCall(checkIncomingCall, 2000);      // 2 sec
Ticker timerGyroscopeAlarm(checkGyroscopeAlarm, 2500);       // 2.5 sec
Ticker timerMagneticLockAlarm(checkMagneticLockAlarm, 1800); // 1.8 sec
// TODO: sound level measuring
// TODO: gyroscope alarm
// TODO: magnetic lock alarm
// TODO: buzzer siren
// TODO: mic switchinng

void setup()
{
  Serial.begin(115200);
  connection.initialize();

  timerSendDataToInternet.start();
  timerCheckIncomingCall.start();
  timerGyroscopeAlarm.start();
  timerMagneticLockAlarm.start();

  meteoData.initializeSensors();

  // set first data for gyroscope and magnetic locks, other in timers..
  gyroscopeController.setData();
  magneticLockController.setData();
  Serial.println("Setup done, send first data");
  sendDataToInternet();
  Serial.println("First data sended, start loop");
}

void loop()
{
  timerSendDataToInternet.update();
  timerCheckIncomingCall.update();
  timerGyroscopeAlarm.update();
  timerMagneticLockAlarm.update();
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
    // gyroscope and magnetic locks data are set in other timer more often, so we have actual data

    Serial.println("Sending data to Blynk");
    connection.sendDataToBlynk(meteoData, powerController, gyroscopeController, magneticLockController);
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

  // TODO: alarm -> connect to GPRS, send notification, run buzzer sound etc..
}

void checkMagneticLockAlarm()
{
  magneticLockController.setData();
  if (!magneticLockController.check())
  {
    connection.alarmMagneticController(magneticLockController);
  }
}