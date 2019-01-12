#include "MeteoData.h"

// SHT3X sensors
Adafruit_SHT31 sht31 = Adafruit_SHT31();

MeteoData::MeteoData()
{
    // one of the sensors is read on the address 0x44, others are set to the address 0x45 etc.
    pinMode(SENSOR_A_PIN, OUTPUT);
    pinMode(SENSOR_B_PIN, OUTPUT);
    //pinMode(SENSOR_C_PIN, OUTPUT);
    digitalWrite(SENSOR_A_PIN, LOW);  // 0x44
    digitalWrite(SENSOR_B_PIN, HIGH); // 0x45
    //digitalWrite(SENSOR_C_PIN, HIGH); // 0x45

    if (!sht31.begin(0x44))
    {
        Serial.println("Could not find a valid SHT31X sensor on oaddress 0x44!");
    }
}

void MeteoData::setData()
{
    digitalWrite(SENSOR_A_PIN, LOW); // 0x44
    sensorA.temperature = sht31.readTemperature();
    sensorA.humidity = sht31.readHumidity();
    digitalWrite(SENSOR_A_PIN, HIGH); // 0x45

    digitalWrite(SENSOR_B_PIN, LOW); // 0x44
    sensorB.temperature = sht31.readTemperature();
    sensorB.humidity = sht31.readHumidity();
    digitalWrite(SENSOR_B_PIN, HIGH); // 0x45

    // TODO sensor C

    // TODO jedna metoda na vypis..
    Serial.print("Sensor A :");
    Serial.print("temperature: " + String(sensorA.temperature) + "°C ");
    Serial.println("humidity: " + String(sensorA.humidity) + "%");
    Serial.print("Sensor B :");
    Serial.print("temperature: " + String(sensorB.temperature) + "°C ");
    Serial.println("humidity: " + String(sensorB.humidity) + "%");
    // TODO sensor C...
}

bool MeteoData::dataAreValid(TempAndHumidity sensor)
{
    return sensor.temperature <= 50.0 && sensor.temperature >= 5.0 &&
           sensor.humidity <= 100.0 && sensor.humidity >= 0.0;
}