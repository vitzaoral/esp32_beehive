#include "InternetConnection.h"
#include <BlynkSimpleSIM800.h>
#include "../../src/settings.cpp"

// SIM800L -> ESP32 wiring to UART2
// SIM800L RX -> ESP32 TX2
// SIM800L TX -> ESP32 RX2

// Hardware Serial - UART2
HardwareSerial gsmSerial(2);

TinyGsm modem(gsmSerial);
Settings settings;

#define DTR_PIN 3

void InternetConnection::initialize()
{
    // Set GSM module baud rate
    gsmSerial.begin(115200, SERIAL_8N1, 16, 17, false);
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
            Serial.println("*** CALL ***");
            // automatically hangup long call
            // now = millis();
            // while (millis() < now + period)
            // {
            // }
            // modem.callHangup();
            // Serial.println("*** BYE !!! ***");
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
        // signal TODO: http://m2msupport.net/m2msupport/atcsq-signal-quality/
        Blynk.virtualWrite(V1, modem.getSignalQuality());

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
    }
    else
    {
        Serial.println("Blynk is not connected");
    }
}
