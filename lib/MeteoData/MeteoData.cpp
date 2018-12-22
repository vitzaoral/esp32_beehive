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
}

bool MeteoData::dataAreValid(TempAndHumidity sensor)
{
    return sensor.temperature <= 50.0 && sensor.temperature >= 5.0 &&
           sensor.humidity <= 100.0 && sensor.humidity >= 0.0;
}