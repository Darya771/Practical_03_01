#include "sensor.h"

Sensor::Sensor(const QString& sensor_name)
    : sensor_name(sensor_name) {
}

// ---------- Gyroscope ----------
Gyroscope::Gyroscope(double initial_value)
    : Sensor("Gyroscope"),
      angular_velocity(initial_value) {
}
double Gyroscope::read() { return angular_velocity; }
void Gyroscope::set_ready(bool value) { ready = value; }

// ---------- Accelerometer ----------
Accelerometer::Accelerometer(double initial_value)
    : Sensor("Accelerometer"),
      acceleration(initial_value) {
}
double Accelerometer::read() { return acceleration; }

// ---------- Barometer ----------
Barometer::Barometer(double initial_value)
    : Sensor("Barometer"),
      pressure(initial_value) {
}
double Barometer::read() { return pressure; }
