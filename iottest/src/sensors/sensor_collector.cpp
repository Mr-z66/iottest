#include "sensors/sensor_collector.h"

#include "core/config.h"
#include "sensors/dht11_sensor.h"
#include "sensors/sgp30_sensor.h"
#include "core/time_utils.h"

SensorCollector::SensorCollector()
    : availability(),
      hw390Sensor(SOIL_MOISTURE_PIN),
      ds18b20Sensor(DS18B20_PIN),
      rainSensor(RAIN_SENSOR_PIN),
      pirSensor(PIR_SENSOR_PIN),
      gy302Sensor() {
}

void SensorCollector::begin() {
    availability.dht11 = (initDHT11() == SENSOR_OK);
    availability.sgp30 = (initSGP30() == SENSOR_OK);
    availability.hw390 = (initHW390() == SENSOR_OK);
    availability.ds18b20 = (ds18b20Sensor.begin() == SENSOR_OK);
    availability.rain = rainSensor.begin();
    availability.pir = (pirSensor.begin() == SENSOR_OK);
    availability.gy302 = (gy302Sensor.begin() == SENSOR_OK);
}

bool SensorCollector::sample(TelemetrySnapshot& snapshot) {
    snapshot = TelemetrySnapshot();
    snapshot.timestamp = TimeUtils::iso8601Now();

    if (availability.dht11) {
        float humidity = 0.0f;
        float temperature = 0.0f;
        if (readDHT11(humidity, temperature) == SENSOR_OK) {
            snapshot.hasHumidity = true;
            snapshot.humidity = humidity;
            snapshot.hasTemperature = true;
            snapshot.temperature = temperature;
        }
    }

    if (availability.sgp30) {
        uint16_t tvoc = 0;
        uint16_t eco2 = 0;
        if (readSGP30(tvoc, eco2) == SENSOR_OK) {
            snapshot.hasTvoc = true;
            snapshot.tvoc = tvoc;
            snapshot.hasCo2 = true;
            snapshot.co2 = eco2;
        }
    }

    if (availability.hw390 && hw390Sensor.readData()) {
        snapshot.hasSoilMoisture = true;
        snapshot.soilMoisture = hw390Sensor.getPercentage();
    }

    if (availability.ds18b20 &&
        ds18b20Sensor.readData() == SENSOR_OK &&
        ds18b20Sensor.isDataValid()) {
        snapshot.hasSoilTemperature = true;
        snapshot.soilTemperature = ds18b20Sensor.getTemperature();
    }

    if (availability.rain && rainSensor.readData()) {
        snapshot.hasRainDetected = true;
        snapshot.rainDetected = rainSensor.isRainDetected();
    }

    if (availability.pir && pirSensor.readData() == SENSOR_OK) {
        snapshot.hasMotionDetected = true;
        snapshot.motionDetected = pirSensor.isMotionDetected();
    }

    if (availability.gy302 &&
        gy302Sensor.readData() == SENSOR_OK &&
        gy302Sensor.isDataValid()) {
        snapshot.hasLight = true;
        snapshot.light = gy302Sensor.getLux();
    }

    return true;
}

SensorAvailabilitySnapshot SensorCollector::getAvailability() const {
    return availability;
}
