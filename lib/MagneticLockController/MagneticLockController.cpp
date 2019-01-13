#include "MagneticLockController.h"

MagneticLockController::MagneticLockController()
{
    // one of the sensors is read on the address 0x1C, others are set to the address 0x1D etc.
    pinMode(LOCK_A_PIN, INPUT);
    pinMode(LOCK_B_PIN, INPUT);
    pinMode(LOCK_C_PIN, INPUT);
}

void MagneticLockController::setData()
{
    Serial.print("Magnetic lock A: ");
    setSensorData(&sensorA, LOCK_A_PIN);
    Serial.print("Magnetic lock B: ");
    setSensorData(&sensorB, LOCK_B_PIN);
    Serial.print("Magnetic lock C: ");
    setSensorData(&sensorC, LOCK_C_PIN);
}

void MagneticLockController::setSensorData(LockData *data, int pin)
{
    if (digitalRead(pin) == HIGH)
    {
        data->locked = true;
        data->status = "OK";
        Serial.println("OK - locked");
    }
    else
    {
        data->locked = false;
        data->status = "NEZAMČENO!";
        Serial.println("UNLOCKED!");
    }
}