#include "InternetConnection.h"
#include <BlynkSimpleTinyGSM.h>
#include "../../src/settings.cpp"

// SIM800L -> ESP32 wiring to UART2
// SIM800L RX -> ESP32 TX2
// SIM800L TX -> ESP32 RX2

// Hardware Serial - UART2
HardwareSerial gsmSerial(2);

Settings settings;

// variables for HTTP access
TinyGsm modem(gsmSerial);

// Attach Blynk virtual serial terminal
WidgetTerminal terminal(V36);

// Siren alarm controller
SirenController sirenController;

// OTA - firmware file name for OTA update on the SPIFFS
const char firmwareFile[] = "/firmware.bin";
// OTA - Number of milliseconds to wait without receiving any data before we give up
const int otaNetworkTimeout = 30 * 1000;
// OTA - Number of milliseconds to wait if no data is available before trying again
const int otakNetworkDelay = 1000;

// alarm notifications are enabled
bool alarmEnabledNotifications = true;

// alarm is enabled
bool alarmIsEnabled = true;

// start OTA update process
bool startOTA = false;

// if siren is turn on/off
bool sirenAlarm = false;

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

// Terminal input
BLYNK_WRITE(V36)
{
    String valueFromTerminal = param.asStr();

    if (String("clear") == valueFromTerminal)
    {
        terminal.clear();
        terminal.println("CLEARED");
        terminal.flush();
    }
    else if (String("update") == valueFromTerminal)
    {
        terminal.clear();
        terminal.println("Start OTA enabled");
        terminal.flush();
        startOTA = true;
    }
    else if (valueFromTerminal != "\n" || valueFromTerminal != "\r" || valueFromTerminal != "")
    {
        terminal.println(String("unknown command: ") + valueFromTerminal);
        terminal.flush();
    }
}

// Siren input
BLYNK_WRITE(V37)
{
    sirenAlarm = param.asInt();
    Serial.println("Siren was " + String(sirenAlarm ? "enabled" : "disabled"));
}

void InternetConnection::initialize()
{
    // setup microphone turn off/on pins
    pinMode(MICROPHONE_A_PIN, OUTPUT);
    pinMode(MICROPHONE_B_PIN, OUTPUT);
    pinMode(MICROPHONE_C_PIN, OUTPUT);

    pinMode(MODEM_RESET_PIN, OUTPUT);

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
    delay(1000);
    digitalWrite(MODEM_RESET_PIN, HIGH);
    delay(400);
    digitalWrite(MODEM_RESET_PIN, LOW);
    delay(400);

    Serial.println("Restarting modem - initial serial, factory default");
    // Set GSM module baud rate
    gsmSerial.begin(115200, SERIAL_8N1, 16, 17, false);
    modem.factoryDefault();

    Serial.println("Restarting modem - doing restart");

    if (!modem.restart())
    {
        Serial.println("Modem FAILED");
        modemReady = false;
    }
    else
    {
        Serial.println("Modem restart OK");
        modemReady = true;
        Blynk.config(modem, settings.blynkAuth);
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
    int callTime = 30000;
   // int pauseTime = 100;

    Serial.println("Microphone A on");
    digitalWrite(MICROPHONE_A_PIN, HIGH);
    delay(callTime);
    digitalWrite(MICROPHONE_A_PIN, LOW);
    // modem.dtmfSend('A', 1000);
    // delay(pauseTime);

    // TODO: switching microphones
    // Serial.println("Microphone B on");
    // digitalWrite(MICROPHONE_B_PIN, HIGH);
    // delay(60000);
    // digitalWrite(MICROPHONE_B_PIN, LOW);
    // modem.dtmfSend('A', 1000);
    // delay(pauseTime);

    // Serial.println("Microphone C on");
    // digitalWrite(MICROPHONE_C_PIN, HIGH);
    // delay(callTime);
    // digitalWrite(MICROPHONE_C_PIN, LOW);

    Serial.println("*** BYE BYE ***");
    modem.callHangup();
}

bool InternetConnection::initializeConnection()
{
    bool result = false;

    if (!modemReady)
    {
        Serial.println("Modem is not ready, can't start Blynk");
    }

    Serial.println("Initialize Blynk connection - modem ready, start connect to GPRS");

    if (modemReady && modem.gprsConnect(settings.apn))
    {
        // TODO: wake up?
        // digitalWrite(DTR_PIN, LOW);
        // delay(100);
        // modem.sleepEnable(false);
        // digitalWrite(DTR_PIN, HIGH);
        Serial.println("Initialize Blynk connection - modem gprs connect OK, start Blynk");
        Blynk.connect();
        Serial.println("Initialize Blynk connection - Blynk OK");
        result = true;
    }
    else
    {
        Serial.println("Can't connect to GPRS");
    }

    if (!result)
    {
        restartModem();
    }
    return result;
}

void InternetConnection::disconnect()
{
    if (modemReady && modem.isGprsConnected())
    {
        // blynk turn off siren
        if (sirenAlarm)
        {
            Blynk.virtualWrite(V37, false);
        }

        modem.gprsDisconnect();
        // TODO: sleep?
        //modem.sleepEnable(true);
        Serial.println("Disconnected OK");
    }
    else
    {
        Serial.println("Disconnected failed - modem wasn't ready");
    }

    // turn off siren alarm
    sirenAlarm = false;
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

        // set SDA/SCL status
        setI2CStatusVersion();

        // magnetic locks data
        setMagneticLockControllerDataToBlynk(magneticLockController);

        // outdoor temperature sensor
        Blynk.virtualWrite(V22, meteoData.sensorOutdoor.humidity);
        Blynk.virtualWrite(V23, meteoData.sensorOutdoor.temperature);
        Blynk.virtualWrite(V24, meteoData.sensorOutdoor.pressure);

        // set alarm info
        setAlarmInfoToBlynk();

        Serial.println("Sending data to Blynk - DONE");
    }
    else
    {
        Serial.println("Blynk is not connected");
    }
}

void InternetConnection::setI2CStatusVersion()
{
    // I2C status - SDA and SCL
    bool SDAisOK = digitalRead(SDA);
    bool SCLsOK = digitalRead(SCL);

    String message = (SDAisOK ? String("SDA OK, ") : String("SDA error, ")) +
                     (SCLsOK ? String("SCL OK, ") : String("SCL error, ")) +
                     String("Version: ") + settings.version;

    Blynk.virtualWrite(V17, message);
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
    else if (quality < 25)
    {
        message = "výborná";
        color = "#008000";
    }
    else {
        message = "vynikající";
        color = "#008000";
    }
    Blynk.virtualWrite(virtualPin, message);

    // increases the time to send data, commented out..
    // Blynk.setProperty(virtualPin, "color", color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// ALARM SECTION
////////////////////////////////////////////////////////////////////////////////////////////////////

void InternetConnection::setAlarmInfoToBlynk()
{
    Blynk.virtualWrite(V32, isAlarm ? "AKTUÁLNÍ ALARM!" : "OK");
    setAlarmCollor(V32, !isAlarm);

    Blynk.virtualWrite(V33, alarmEnabledNotifications ? "Alarm notifikace zapnuty" : "Alarm notifikace vypnuty");
    setAlarmCollor(V33, alarmEnabledNotifications);

    Blynk.virtualWrite(V34, alarmIsEnabled ? "Alarm zapnut" : "Alarm vypnut");
    setAlarmCollor(V34, alarmIsEnabled);

    if (isAlarm)
    {
        Serial.println("\n ALARM \n");
    }
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
    // increases the time to send data, commented out..
    // Blynk.setProperty(virtualPin, "color", String(isOk ? "#008000" : "#ff0000"));
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

void InternetConnection::checkSirenAlarm()
{
    if (sirenAlarm)
    {
        sirenController.runSiren();
        printlnToTerminal("Siréna spuštěna");
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

////////////////////////////////////////////////////////////////////////////////////////////////////
/// OTA SECTION
////////////////////////////////////////////////////////////////////////////////////////////////////

void InternetConnection::checkNewVersionAndUpdate()
{
    if (!startOTA)
    {
        return;
    }
    else
    {
        startOTA = false;
    }

    terminal.println("Start OTA, check GPRS");

    if (!modem.isGprsConnected())
    {
        Serial.println("GPRS not connected!");
        return;
    }

    int statusCode = 0;

    // need second modem instance for http client
    TinyGsm modemHttpClient(gsmSerial);
    TinyGsmClient client(modemHttpClient);
    HttpClient http(client, settings.firmwareUrlBase, 80);

    String fwVersionURL = String(settings.firmwareFileName);
    fwVersionURL.concat(String(settings.firmwareVersionFileNameExt));

    Serial.println("GPRS OK, making GET request for firmware version");

    statusCode = http.get(fwVersionURL);
    if (statusCode == 0)
    {
        statusCode = http.responseStatusCode();
        if (statusCode == 200)
        {
            String version = http.responseBody();
            Serial.println("Version: " + version);

            if (String(settings.version) != version)
            {
                Serial.println("!!! START OTA UPDATE !!!");
                updateFirmware(http);
            }
            else
            {
                Serial.println("Already on the latest version");
            }
        }
        else
        {
            Serial.println("Failed verify version from server, status code: " + String(statusCode));
        }
    }
    else
    {
        Serial.println("Failed verify version from server, status code: " + String(statusCode));
    }

    Serial.println("Restart after OTA, bye");
    delay(1000);
    ESP.restart();
}

void InternetConnection::updateFirmware(HttpClient http)
{
    // prepare and format SPIFFS
    Serial.println("Begin SPIFFS");
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    Serial.println("Format SPIFFS");
    SPIFFS.format();
    Serial.println("Format SPIFFS done");

    String fwFileURL = String(settings.firmwareFileName);
    fwFileURL.concat(String(settings.firmwareFileNameExt));

    Serial.println("Download file from: " + fwFileURL);

    int statusCode = http.get(fwFileURL);
    if (statusCode == 0)
    {
        statusCode = http.responseStatusCode();
        if (statusCode == 200)
        {
            Serial.println("Getting new firmware file OK");

            int contentLength = http.contentLength();
            Serial.println("Content length is: " + String(contentLength));

            unsigned long timeoutStart = millis();
            char c;
            uint32_t readLength = 0;

            printPercent(readLength, contentLength);

            // create firmware file in SPIFFS
            File file = SPIFFS.open(firmwareFile, FILE_APPEND);

            // Whilst we haven't timed out & haven't reached the end of the body
            while ((http.connected() || http.available()) &&
                   (!http.endOfBodyReached()) &&
                   ((millis() - timeoutStart) < otaNetworkTimeout))
            {
                if (http.available())
                {
                    c = http.read();
                    if (!file.print(c))
                    {
                        Serial.println("Failed to append to firmware file!!");
                    }

                    readLength++;
                    if (readLength % (contentLength / 13) == 0)
                    {
                        printPercent(readLength, contentLength);
                    }

                    // We read something, reset the timeout counter
                    timeoutStart = millis();
                }
                else
                {
                    // We haven't got any data, so let's pause to allow some to arrive
                    delay(otakNetworkDelay);
                }
            }

            Serial.println("Download done, disconnect");
            file.close();
            http.stop();
            Serial.println("Start update firmware");
            updateFromFS();
        }
        else
        {
            Serial.println("Getting response failed: " + String(statusCode));
        }
    }
    else
    {
        Serial.println("Connect failed: " + String(statusCode));
    }
}

void InternetConnection::performUpdate(Stream &updateSource, size_t updateSize)
{
    if (Update.begin(updateSize))
    {
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize)
        {
            Serial.println("Ready for update: " + String(written) + " bits successfully");
        }
        else
        {
            Serial.println("Firmware file size mismatch : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }

        if (Update.end())
        {
            Serial.println("OTA was done succesfully");
            if (Update.isFinished())
            {
                Serial.println("OTA ended succesfully.");
            }
            else
            {
                Serial.println("Failed on the OTA process");
            }
        }
        else
        {
            Serial.println("OTA Error #: " + String(Update.getError()));
        }
    }
    else
    {
        Serial.println("OTA problem - can't begin update process");
    }
}

void InternetConnection::updateFromFS()
{
    File updateBin = SPIFFS.open(firmwareFile);
    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            Serial.println("Error, shouldn't be directory, expected file");
            updateBin.close();
            return;
        }

        size_t updateSize = updateBin.size();

        if (updateSize > 0)
        {
            Serial.println("Start firmware update");
            performUpdate(updateBin, updateSize);
        }
        else
        {
            Serial.println("Error, firmware file has zero size");
        }

        Serial.println("Remove firmware file and restart ESP.");
        updateBin.close();
        SPIFFS.remove(firmwareFile);
        disconnect();
        ESP.restart();
    }
    else
    {
        Serial.println("File with firmware update not found");
    }
}

void InternetConnection::printPercent(uint32_t readLength, uint32_t contentLength)
{
    // If we know the total length
    if (contentLength != -1)
    {
        Serial.println("\r " + String((100.0 * readLength) / contentLength) + "%");
    }
    else
    {
        Serial.println(String(readLength));
    }
}

/// Println message to serial and Blynk terminal
void InternetConnection::printlnToTerminal(String message)
{
    Serial.println(message);
    terminal.println(message);
    terminal.flush();
}