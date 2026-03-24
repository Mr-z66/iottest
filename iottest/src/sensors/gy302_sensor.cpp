#include "sensors/gy302_sensor.h"

GY302Sensor::GY302Sensor()
    : lightMeter(), lux(0.0f), initialized(false), dataValid(false) {
}

int GY302Sensor::begin() {
    Wire.begin(GY302_SDA, GY302_SCL);
    delay(100);

    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, GY302_I2C_ADDRESS, &Wire)) {
        initialized = false;
        dataValid = false;
        return SENSOR_ERROR;
    }

    initialized = true;
    dataValid = false;

    Serial.printf("GY-302 initialized on I2C SDA=GPIO%d, SCL=GPIO%d, address=0x%02X\n",
                  GY302_SDA,
                  GY302_SCL,
                  GY302_I2C_ADDRESS);

    return SENSOR_OK;
}

int GY302Sensor::readData() {
    if (!initialized) {
        return SENSOR_DISABLED;
    }

    float reading = lightMeter.readLightLevel();
    if (reading < 0) {
        dataValid = false;
        return SENSOR_ERROR;
    }

    lux = reading;
    dataValid = true;
    return SENSOR_OK;
}

float GY302Sensor::getLux() const {
    return lux;
}

bool GY302Sensor::isDataValid() const {
    return dataValid;
}

void GY302Sensor::printDebugInfo() const {
    Serial.printf("DEBUG: GY-302 light level = %.2f lux\n", lux);
}
