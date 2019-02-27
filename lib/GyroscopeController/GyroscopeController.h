#ifndef __GyroscopeController_H
#define __GyroscopeController_H

#include <Arduino.h>
#include <SparkFun_MMA8452Q.h>

#define GYROSCOPE_A_PIN 14
#define GYROSCOPE_B_PIN 12
#define GYROSCOPE_C_PIN 13

struct GyroscopeData
{
    float x;
    float y;
    float z;
    String orientation;
};

class GyroscopeController
{
  public:
    GyroscopeController();
    GyroscopeData sensorA;
    GyroscopeData sensorB;
    GyroscopeData sensorC;
    bool isOk();
    void setData();
    void initializeSensors();

  private:
    String setOrientation();
    void setSensorData(GyroscopeData *data);
    void printCalculatedAccels(GyroscopeData *data);
};

#endif