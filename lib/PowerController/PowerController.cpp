#include "PowerController.h"

Adafruit_INA219 ina219;

PowerController::PowerController()
{
    ina219.begin();
    ina219.setCalibration_16V_400mA();
}

void PowerController::setData()
{
    shuntVoltage = ina219.getShuntVoltage_mV();
    busVoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    loadVoltage = busVoltage + (shuntVoltage / 1000);

    Serial.print("Bus Voltage:   ");
    Serial.print(busVoltage);
    Serial.println(" V");
    Serial.print("Shunt Voltage: ");
    Serial.print(shuntVoltage);
    Serial.println(" mV");
    Serial.print("Load Voltage:  ");
    Serial.print(loadVoltage);
    Serial.println(" V");
    Serial.print("Current:       ");
    Serial.print(current_mA);
    Serial.println(" mA");
    Serial.print("Power:         ");
    Serial.print(power_mW);
    Serial.println(" mW");
}