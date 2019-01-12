#include "InternetConnection.h"
#include <BlynkSimpleSIM800.h>
#include "../../src/settings.cpp"

// SIM800L -> ESP32 wiring to UART2
// SIM800L RX -> ESP32 TX2
// SIM800L TX -> ESP32 RX2

// Hardware Serial - UART2
HardwareSerial gsmSerial(2);

// TODO: OTA over SIM800L https://github.com/vshymanskyy/TinyGSM/issues/133#issuecomment-372327106

TinyGsm modem(gsmSerial);
Settings settings;

// TODO: PIN for sleep/wake modem..unused - delete?
#define DTR_PIN 3

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
        Serial.println("Restarting, bye..");
        ESP.restart();
    }
}

void InternetConnection::initialize()
{
    // Set GSM module baud rate
    gsmSerial.begin(115200, SERIAL_8N1, 16, 17, false);
    // TODO: sleep/wake upp https://www.raviyp.com/embedded/223-sim900-sim800-sleep-mode-at-commands
    // pinMode(DTR_PIN, OUTPUT);
    // digitalWrite(DTR_PIN, HIGH);
    restartModem();
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
        }
    }
    else
    {
        restartModem();
    }
}

bool InternetConnection::initializeConnection()
{
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

void InternetConnection::sendDataToBlynk(MeteoData meteoData, PowerController powerController)
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
    }
    else
    {
        Serial.println("Blynk is not connected");
    }
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
    if (quality < 15)
    {
        message = "dostačující";
        color = "#cc8400";
    }
    if (quality < 20)
    {
        message = "dobrá";
        color = "#93bd38";
    }
    if (quality <= 30)
    {
        message = "výborná";
        color = "#008000";
    }
    Blynk.virtualWrite(virtualPin, message);
    Blynk.setProperty(virtualPin, "color", color);
}
