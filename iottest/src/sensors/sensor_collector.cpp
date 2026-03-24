#include "sensors/sensor_collector.h"

#include "core/config.h"
#include "sensors/dht11_sensor.h"
#include "sensors/sgp30_sensor.h"
#include "core/time_utils.h"

#if USE_MOCK_DATA
#include <random>
#endif

SensorCollector::SensorCollector()
    : availability(),
      hw390Sensor(SOIL_MOISTURE_PIN),
      ds18b20Sensor(DS18B20_PIN),
      rainSensor(RAIN_SENSOR_PIN),
      pirSensor(PIR_SENSOR_PIN),
      gy302Sensor() {
}

void SensorCollector::begin() {
#if USE_MOCK_DATA
    // Mock mode: all sensors are available without hardware init
    availability.dht11 = true;
    availability.sgp30 = true;
    availability.hw390 = true;
    availability.ds18b20 = true;
    availability.rain = true;
    availability.pir = true;
    availability.gy302 = true;
#else
    // Real mode: initialize hardware sensors
    availability.dht11 = (initDHT11() == SENSOR_OK);
    availability.sgp30 = (initSGP30() == SENSOR_OK);
    availability.hw390 = (initHW390() == SENSOR_OK);
    availability.ds18b20 = (ds18b20Sensor.begin() == SENSOR_OK);
    availability.rain = rainSensor.begin();
    availability.pir = (pirSensor.begin() == SENSOR_OK);
    availability.gy302 = (gy302Sensor.begin() == SENSOR_OK);
#endif
}

bool SensorCollector::sample(TelemetrySnapshot& snapshot) {
    snapshot = TelemetrySnapshot();
    snapshot.timestamp = TimeUtils::iso8601Now();

#if USE_MOCK_DATA
    // Mock data mode - generate simulated sensor values
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> tempDist(20.0f, 30.0f);
    static std::uniform_real_distribution<float> humDist(40.0f, 70.0f);
    static std::uniform_real_distribution<float> soilMoistDist(30.0f, 80.0f);
    static std::uniform_real_distribution<float> soilTempDist(18.0f, 25.0f);
    static std::uniform_real_distribution<float> lightDist(100.0f, 2000.0f);
    static std::uniform_int_distribution<uint16_t> co2Dist(400, 1000);
    static std::uniform_int_distribution<uint16_t> tvocDist(10, 200);
    static std::uniform_int_distribution<int> boolDist(0, 1);

    snapshot.hasTemperature = true;
    snapshot.temperature = tempDist(gen);
    snapshot.hasHumidity = true;
    snapshot.humidity = humDist(gen);
    snapshot.hasSoilMoisture = true;
    snapshot.soilMoisture = soilMoistDist(gen);
    snapshot.hasSoilTemperature = true;
    snapshot.soilTemperature = soilTempDist(gen);
    snapshot.hasLight = true;
    snapshot.light = lightDist(gen);
    snapshot.hasCo2 = true;
    snapshot.co2 = co2Dist(gen);
    snapshot.hasTvoc = true;
    snapshot.tvoc = tvocDist(gen);
    snapshot.hasRainDetected = true;
    snapshot.rainDetected = boolDist(gen) == 1;
    snapshot.hasMotionDetected = true;
    snapshot.motionDetected = boolDist(gen) == 1;

    // All sensors are "available" in mock mode
    availability.dht11 = true;
    availability.sgp30 = true;
    availability.hw390 = true;
    availability.ds18b20 = true;
    availability.rain = true;
    availability.pir = true;
    availability.gy302 = true;

    return true;
#else
    // Real sensor reading mode
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
#endif
}

SensorAvailabilitySnapshot SensorCollector::getAvailability() const {
    return availability;
}
