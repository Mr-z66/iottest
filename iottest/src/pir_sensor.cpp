#include "pir_sensor.h"

PIRSensor::PIRSensor(int digitalPin)
    : pin(digitalPin), motionDetected(false), digitalValue(LOW) {
}

int PIRSensor::begin() {
    pinMode(pin, INPUT);
    updateReading();

    Serial.printf("PIR sensor initialized on GPIO%d\n", pin);
    Serial.println("Note: HC-SR501 sensor - HIGH = motion detected, LOW = idle");

    return SENSOR_OK;
}

int PIRSensor::readData() {
    updateReading();
    return SENSOR_OK;
}

bool PIRSensor::isMotionDetected() const {
    return motionDetected;
}

bool PIRSensor::isIdle() const {
    return !motionDetected;
}

int PIRSensor::getDigitalValue() const {
    return digitalValue;
}

void PIRSensor::printDebugInfo() const {
    Serial.printf("DEBUG: PIR sensor GPIO%d = %d (Motion: %s)\n",
                  pin,
                  digitalValue,
                  motionDetected ? "true" : "false");
}

void PIRSensor::updateReading() {
    digitalValue = digitalRead(pin);
    motionDetected = (digitalValue == HIGH);
}
