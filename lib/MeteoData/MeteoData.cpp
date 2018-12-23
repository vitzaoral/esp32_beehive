#include "MeteoData.h"

// DHT22 sensors
DHTesp dhtA;
DHTesp dhtB;

MeteoData::MeteoData()
{
    dhtA.setup(18, DHTesp::DHT22);
    dhtB.setup(19, DHTesp::DHT22);
}

void MeteoData::setData()
{
    sensorA = dhtA.getTempAndHumidity();
    sensorB = dhtB.getTempAndHumidity();

    // TODO jedna metoda na vypis..
    Serial.print("Sensor A :");
    Serial.print("temperature: " + String(sensorA.temperature) + "°C ");
    Serial.println("humidity: " + String(sensorA.humidity) + "%");
    Serial.print("Sensor B :");
    Serial.print("temperature: " + String(sensorB.temperature) + "°C ");
    Serial.println("humidity: " + String(sensorB.humidity) + "%");
}

bool MeteoData::dataAreValid(TempAndHumidity sensor)
{
    return sensor.temperature <= 50.0 && sensor.temperature >= 5.0 &&
           sensor.humidity <= 100.0 && sensor.humidity >= 0.0;
}