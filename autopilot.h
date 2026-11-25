#ifndef AUTOPILOT_H
#define AUTOPILOT_H

#include <QObject>
#include <QString>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"

// Модель с сигналами: ничего не знает об окне.
class Autopilot : public QObject {
    Q_OBJECT
public:
    Autopilot(UAV* vehicle,
              const std::vector<Sensor*>& sensors,
              NavigationSystem* navigation,
              FlightController* controller,
              QObject* parent = nullptr);

    // Команда — вниз: окно меняет цель только через публичный метод
    void set_target_altitude(double value);
    double target_altitude() const { return target; }

public slots:
    void run_cycle();

signals:
    void telemetry_updated(double altitude, double error, double control);
    void fault_detected(const QString& reason);

private:
    UAV* vehicle;
    std::vector<Sensor*> sensors;
    NavigationSystem* navigation;
    FlightController* controller;
    double target = 200.0;          // целевая высота, м
    const double dt = 0.1;          // период цикла, с (10 Гц)
};

#endif // AUTOPILOT_H
