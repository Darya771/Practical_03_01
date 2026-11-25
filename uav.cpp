#include "uav.h"

#include <algorithm>

UAV::UAV(double battery_percent, double start_altitude)
    : current_altitude(start_altitude),
      battery_percent(battery_percent) {
}

void UAV::apply_control(double control, double dt) {
    double climb_rate = std::clamp(control, -max_climb_rate, max_climb_rate);
    current_altitude += climb_rate * dt;
    if (current_altitude < 0.0) {
        current_altitude = 0.0;     // ниже земли не опускаемся
    }
}

// ---------- Quadcopter ----------
Quadcopter::Quadcopter(double battery, double altitude)
    : MultirotorUAV(battery, altitude) {
}
void Quadcopter::takeoff() { flying = true; }          // вертикальный взлёт
void Quadcopter::land()    { flying = false; }         // вертикальная посадка
QString Quadcopter::type() const { return "Quadcopter"; }
int Quadcopter::motor_count() const { return 4; }

// ---------- Hexacopter ----------
Hexacopter::Hexacopter(double battery, double altitude)
    : MultirotorUAV(battery, altitude) {
}
void Hexacopter::takeoff() { flying = true; }
void Hexacopter::land()    { flying = false; }
QString Hexacopter::type() const { return "Hexacopter"; }
int Hexacopter::motor_count() const { return 6; }

// ---------- FixedWingUAV ----------
FixedWingUAV::FixedWingUAV(double battery, double altitude)
    : UAV(battery, altitude) {
    max_climb_rate = 5.0;           // самолёт набирает высоту медленнее
}
void FixedWingUAV::takeoff() { flying = true; }        // разбег по полосе
void FixedWingUAV::land()    { flying = false; }       // посадка планированием
QString FixedWingUAV::type() const { return "Fixed-wing UAV"; }
