#include "InternetConnection.h"
#include <BlynkSimpleTinyGSM.h>
#include "../../src/settings.cpp"

// SIM800L -> ESP32 wiring to UART2
// SIM800L RX -> ESP32 TX2
// SIM800L TX -> ESP32 RX2

// Hardware Serial - UART2
HardwareSerial gsmSerial(2);

// TODO: OTA over SIM800L https://github.com/vshymanskyy/TinyGSM/issues/133#issuecomment-372327106

TinyGsm modem(gsmSerial);
Settings settings;

// alarm notifications are enabled
bool alarmEnabledNotifications = true;

// alarm is enabled
bool alarmIsEnabled = true;

// TODO: PIN for sleep/wake modem..unused - delete?
#define DTR_PIN 3

#define SDA 21
#define SCL 22

// need lot of blynk virtual pins :)
#define BLYNK_USE_128_VPINS

// Synchronize settings from Blynk server with device when internet is connected
BLYNK_CONNECTED()
{
    Blynk.syncAll();
}

// Restart ESP
BLYNK_WRITE(V0)
{
    if (param.asInt())
    {
        Blynk.virtualWrite(V0, false);

        // TODO: refactor, odstranit? https://github.com/espressif/arduino-esp32/issues/1563#issuecomment-401560601
        Serial.println("Check I2C");
        if (!digitalRead(SDA) || !digitalRead(SCL))
        { // bus in busy state
            log_w("invalid state sda=%d, scl=%d\n", digitalRead(SDA), digitalRead(SCL));
            Serial.print("invalid state SDA: ");
            Serial.println(digitalRead(SDA));
            Serial.print("invalid state SCL: ");
            Serial.println(digitalRead(SCL));
            digitalWrite(SDA, HIGH);
            digitalWrite(SCL, HIGH);
            delayMicroseconds(5);
            digitalWrite(SDA, HIGH);
            for (uint8_t a = 0; a < 9; a++)
            {
                delayMicroseconds(5);
                digitalWrite(SCL, LOW);
                delayMicroseconds(5);
                digitalWrite(SCL, HIGH);
                if (digitalRead(SDA))
                { // bus recovered, all done. resync'd with slave
                    break;
                }
            }
        }
        else
        {
            Serial.println("I2C OK");
        }
        Serial.println("Restarting, bye..");
        ESP.restart();
    }
}

// Turn on/off alarm notifications
BLYNK_WRITE(V30)
{
    alarmEnabledNotifications = param.asInt();
    Serial.println("Alarm notifications was " + String(alarmEnabledNotifications ? "enabled" : "disabled"));
}

// Turn on/off alarm
BLYNK_WRITE(V31)
{
    alarmIsEnabled = param.asInt();
    Serial.println("Alarm was " + String(alarmIsEnabled ? "enabled" : "disabled"));
}

void InternetConnection::initialize()
{
    // setup microphone turn off/on pins
    pinMode(MICROPHONE_A_PIN, OUTPUT);
    pinMode(MICROPHONE_B_PIN, OUTPUT);
    pinMode(MICROPHONE_C_PIN, OUTPUT);

    // Set GSM module baud rate
    gsmSerial.begin(115200, SERIAL_8N1, 16, 17, false);
    // TODO: sleep/wake upp https://www.raviyp.com/embedded/223-sim900-sim800-sleep-mode-at-commands
    // pinMode(DTR_PIN, OUTPUT);
    // digitalWrite(DTR_PIN, HIGH);
    restartModem();

    // true if is actual alarm - run Blynk.run
    isAlarm = false;
}

void InternetConnection::restartModem()
{
    Serial.println("Restarting modem");
    if (!modem.init())
    {
        Serial.println("Modem FAILED");
        modemReady = false;
    }
    else
    {
        Serial.println("Modem restart OK");
        modemReady = true;
        Serial.println("Modem info: " + modem.getModemInfo());
    }
}

void InternetConnection::checkIncomingCall()
{
    if (modemReady)
    {
        if (modem.callAnswer())
        {
            Serial.println("*** INCOMING CALL ***");
            processIncomingCall();
        }
    }
    else
    {
        restartModem();
    }
}

void InternetConnection::processIncomingCall()
{
    Serial.println("Microphone A on");
    digitalWrite(MICROPHONE_A_PIN, HIGH);
    delay(5000);
    digitalWrite(MICROPHONE_A_PIN, LOW);
    delay(1000);

    Serial.println("Microphone B on");
    digitalWrite(MICROPHONE_B_PIN, HIGH);
    delay(5000);
    digitalWrite(MICROPHONE_B_PIN, LOW);
    delay(1000);

    Serial.println("Microphone C on");
    digitalWrite(MICROPHONE_C_PIN, HIGH);
    delay(5000);
    digitalWrite(MICROPHONE_C_PIN, LOW);

    Serial.println("*** BYE BYE ***");
    modem.callHangup();
}

bool InternetConnection::initializeConnection()
{
    Serial.println("Initialize Blynk connection");
    if (modemReady)
    {
        // TODO: wake up?
        // digitalWrite(DTR_PIN, LOW);
        // delay(100);
        // modem.sleepEnable(false);
        // digitalWrite(DTR_PIN, HIGH);
        Blynk.begin(settings.blynkAuth, modem, "internet", "", "");
        return true;
    }
    else
    {
        Serial.println("Modem is not ready, can't start Blynk");
        return false;
    }
}

void InternetConnection::disconnect()
{
    if (modemReady)
    {
        Blynk.disconnect();
        modem.gprsDisconnect();
        // TODO: sleep?
        //modem.sleepEnable(true);
        Serial.println("Disconnected OK");
    }
    else
    {
        Serial.println("Disconnected failed - modem wasn't ready");
    }
}

void InternetConnection::sendDataToBlynk(
    MeteoData meteoData,
    PowerController powerController,
    GyroscopeController gyroscopeController,
    MagneticLockController magneticLockController)
{
    // create data to send to Blynk
    if (Blynk.connected())
    {
        int signalQuality = modem.getSignalQuality();
        Blynk.virtualWrite(V1, signalQuality);

        // battery data
        Blynk.virtualWrite(V2, modem.getBattPercent());
        float battVoltage = modem.getBattVoltage() / 1000.0F;
        Blynk.virtualWrite(V3, battVoltage);

        // meteo data A
        Blynk.virtualWrite(V4, meteoData.sensorA.humidity);
        Blynk.virtualWrite(V5, meteoData.sensorA.temperature);

        // meteo data B
        Blynk.virtualWrite(V6, meteoData.sensorB.humidity);
        Blynk.virtualWrite(V7, meteoData.sensorB.temperature);

        // solar power data
        Blynk.virtualWrite(V8, powerController.loadVoltage);
        Blynk.virtualWrite(V9, powerController.current_mA);
        Blynk.virtualWrite(V10, powerController.power_mW);

        // setup signal quality decription
        getSignalQualityDescription(V11, signalQuality);

        // meteo data C
        Blynk.virtualWrite(V12, meteoData.sensorC.humidity);
        Blynk.virtualWrite(V13, meteoData.sensorC.temperature);

        // gyroscope data
        setGyroscopeControllerDataToBlynk(gyroscopeController);

        // I2C status - SDA
        // TODO: refactor..barvy kod atd...
        if (!digitalRead(SDA))
        {
            Blynk.virtualWrite(V17, "SDA error");
        }
        else
        {
            Blynk.virtualWrite(V17, "SDA OK");
        }
        if (!digitalRead(SCL))
        {
            Blynk.virtualWrite(V18, "SCL error");
        }
        else
        {
            Blynk.virtualWrite(V18, "SCL OK");
        }

        // magnetic locks data
        setMagneticLockControllerDataToBlynk(magneticLockController);

        // outdoor temperature sensor
        Blynk.virtualWrite(V22, meteoData.sensorOutdoor.humidity);
        Blynk.virtualWrite(V23, meteoData.sensorOutdoor.temperature);
        Blynk.virtualWrite(V24, meteoData.sensorOutdoor.pressure);

        // set alarm info
        setAlarmInfoToBlynk();
    }
    else
    {
        Serial.println("Blynk is not connected");
    }
}

void InternetConnection::setGyroscopeControllerDataToBlynk(GyroscopeController gyroscopeController)
{
    Blynk.virtualWrite(V14, gyroscopeController.sensorA.orientation);
    setAlarmCollor(V14, gyroscopeController.sensorA.isOk);

    Blynk.virtualWrite(V15, gyroscopeController.sensorB.orientation);
    setAlarmCollor(V15, gyroscopeController.sensorB.isOk);

    Blynk.virtualWrite(V16, gyroscopeController.sensorC.orientation);
    setAlarmCollor(V16, gyroscopeController.sensorC.isOk);

    setAlarmInfoToBlynk();
}

void InternetConnection::setMagneticLockControllerDataToBlynk(MagneticLockController magneticLockController)
{
    Blynk.virtualWrite(V19, magneticLockController.sensorA.status);
    setAlarmCollor(V19, magneticLockController.sensorA.locked);

    Blynk.virtualWrite(V20, magneticLockController.sensorB.status);
    setAlarmCollor(V20, magneticLockController.sensorB.locked);

    Blynk.virtualWrite(V21, magneticLockController.sensorC.status);
    setAlarmCollor(V21, magneticLockController.sensorC.locked);

    setAlarmInfoToBlynk();
}

// Signal quality description http://m2msupport.net/m2msupport/atcsq-signal-quality/
void InternetConnection::getSignalQualityDescription(int virtualPin, int quality)
{
    String color = "#ff0000";
    String message = "neznámá";

    if (quality < 10)
    {
        message = "mizivá";
        color = "#ff0000";
    }
    else if (quality < 15)
    {
        message = "dostačující";
        color = "#cc8400";
    }
    else if (quality < 20)
    {
        message = "dobrá";
        color = "#93bd38";
    }
    else if (quality <= 30)
    {
        message = "výborná";
        color = "#008000";
    }
    Blynk.virtualWrite(virtualPin, message);
    Blynk.setProperty(virtualPin, "color", color);
}

////////////////////
/// ALARM SECTION
////////////////////

void InternetConnection::setAlarmInfoToBlynk()
{
    Blynk.virtualWrite(V32, isAlarm ? "AKTUÁLNÍ ALARM!" : "OK");
    setAlarmCollor(V32, !isAlarm);

    Blynk.virtualWrite(V33, alarmEnabledNotifications ? "Alarm notifikace zapnuty" : "Alarm notifikace vypnuty");
    setAlarmCollor(V33, alarmEnabledNotifications);

    Blynk.virtualWrite(V34, alarmIsEnabled ? "Alarm zapnut" : "Alarm vypnut");
    setAlarmCollor(V34, alarmIsEnabled);
}

void InternetConnection::blynkRunIfAlarm()
{
    if (alarmIsEnabled && isAlarm)
    {
        Blynk.run();
    }
}

void InternetConnection::setAlarmCollor(int virtualPin, bool isOk)
{
    Blynk.setProperty(virtualPin, "color", String(isOk ? "#008000" : "ff0000"));
}

void InternetConnection::setMagneticLockControllerDataToBlynkIfAlarm(MagneticLockController magneticLockController)
{
    if (isAlarm)
    {
        setMagneticLockControllerDataToBlynk(magneticLockController);
    }
}

void InternetConnection::setGyroscopeControllerDataToBlynkIfAlarm(GyroscopeController gyroscopeController)
{
    if (isAlarm)
    {
        setGyroscopeControllerDataToBlynk(gyroscopeController);
    }
}

void InternetConnection::alarmMagneticController(MagneticLockController magneticLockController)
{
    if (!alarmIsEnabled)
    {
        return;
    }

    Serial.println("\n!!! Magnetic alarm !!!\n");

    if (!Blynk.connected())
    {
        initializeConnection();
    }

    if (Blynk.connected())
    {
        setMagneticLockControllerDataToBlynk(magneticLockController);

        if (alarmEnabledNotifications)
        {
            Blynk.notify("! ALARM ! Magnetický zámek je otevřen: " + magneticLockController.getAlarmMessage());
        }
    }
    else
    {
        // TODO: No Blynk connection, try send SMS or call?
        // res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
        // res = modem.callNumber(CALL_TARGET);
        Serial.println("ALARM but can't connected to Blynk");
    }
}

void InternetConnection::alarmGyroscopeController(GyroscopeController gyroscopeController)
{
    if (!alarmIsEnabled)
    {
        return;
    }

    Serial.println("\n!!! Gyroscope alarm !!!\n");

    if (!Blynk.connected())
    {
        initializeConnection();
    }

    if (Blynk.connected())
    {
        setGyroscopeControllerDataToBlynk(gyroscopeController);

        if (alarmEnabledNotifications)
        {
            Blynk.notify("! ALARM ! Gyroskop není ve správné poloze: " + gyroscopeController.getAlarmMessage());
        }
    }
    else
    {
        // TODO: No Blynk connection, try send SMS or call?
        // res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
        // res = modem.callNumber(CALL_TARGET);
        Serial.println("ALARM but can't connected to Blynk");
    }
}