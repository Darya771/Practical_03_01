#include "autopilot.h"

Autopilot::Autopilot(UAV* vehicle,
                     const std::vector<Sensor*>& sensors,
                     NavigationSystem* navigation,
                     FlightController* controller,
                     QObject* parent)
    : QObject(parent),
      vehicle(vehicle),
      sensors(sensors),
      navigation(navigation),
      controller(controller) {
}

void Autopilot::set_target_altitude(double value) {
    if (value >= 0.0) {
        target = value;
    }
}

void Autopilot::run_cycle() {
    // 1. Аппарат
    if (vehicle == nullptr) {
        emit fault_detected("vehicle is not set");
        return;
    }
    // 2. Сенсоры
    for (Sensor* sensor : sensors) {
        if (sensor == nullptr || !sensor->is_ready()) {
            emit fault_detected(QString("sensor failure: ")
                                + (sensor ? sensor->name() : QString("unknown")));
            return;
        }
    }
    // 3. Навигация
    if (navigation == nullptr || !navigation->is_valid()) {
        emit fault_detected("navigation data invalid");
        return;
    }
    // Контроллер
    if (controller == nullptr) {
        emit fault_detected("flight controller is not set");
        return;
    }
    // 4. Позиция
    Position position = navigation->get_position();
    if (position.altitude < 0.0) {
        emit fault_detected("invalid navigation position");
        return;
    }
    // 5. Ошибка
    double error = target - position.altitude;
    // 6. Управление
    double control = controller->calculate_control(error, dt);
    // Отклик аппарата на управление (упрощённая модель полёта)
    vehicle->apply_control(control, dt);
    // 7. Факт — вверх: рассылаем телеметрию
    emit telemetry_updated(position.altitude, error, control);
}
