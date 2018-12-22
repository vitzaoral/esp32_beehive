#include "InternetConnection.h"

// SIM800L -> ESP32 wiring to UART2
// SIM800L RX -> ESP32 TX2
// SIM800L TX -> ESP32 RX2

// Hardware Serial - UART2
HardwareSerial gsmSerial(2);

TinyGsm modem(gsmSerial);

InternetConnection::InternetConnection()
{
    // Set GSM module baud rate
    gsmSerial.begin(115200, SERIAL_8N1, 16, 17, false);

    if (!modem.restart())
    {
        Serial.println("FAILED");
    }
    else
    {
        Serial.println("Modem restart done");
    }

    String modemInfo = modem.getModemInfo();
    Serial.print("Modem info: ");
    Serial.println(modemInfo);
}