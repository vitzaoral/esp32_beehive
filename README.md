# esp32_beehive
Beehive online monitoring based on the ESP32, SIM800L and others sensors.

> To build a project, you need to download all the necessary libraries and create the *settings.cpp* file in the *src* folder:
```c++
// Project settings
struct Settings
{
    const char *blynkAuth = "You Blynk Auth";
    const char *version = "1.0.0";
    const char *firmawareUrlBase = "http://your-address.com/";
    const char *firmwareFileName = "bee_hive";
    const char *firmwareVersionFileName = ".version";
};
```

### Currents list:

* [ESP32 WROOM-32](https://www.aliexpress.com/item/ESP32-ESP-32-ESP32S-ESP-32S-CP2102-Wireless-WiFi-Bluetooth-Development-Board-Micro-USB-Dual-Core/32867696371.html)
* TODO

### Schema:
![Schema](https://github.com/vitzaoral/metheo-station/blob/master/schema/meteo_schema.jpg)

### PCB circuit:
* TODO

### Blynk:
* TODO
