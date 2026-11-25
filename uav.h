#ifndef UAV_H
#define UAV_H

#include <QString>

// ===== Модуль «Аппарат» =====
// Абстрактный базовый класс: контракт для всех БПЛА.
class UAV {
public:
    UAV(double battery_percent, double start_altitude);
    virtual ~UAV() = default;

    virtual void takeoff() = 0;
    virtual void land() = 0;
    virtual QString type() const = 0;

    // Наблюдатели (const) — не меняют объект
    double altitude() const { return current_altitude; }
    double battery() const { return battery_percent; }
    bool is_flying() const { return flying; }

    // Упрощённая «физика»: управляющий сигнал = вертикальная скорость, м/с
    void apply_control(double control, double dt);

protected:
    double current_altitude;   // наследники видят, внешний код — нет
    double battery_percent;
    bool flying = false;
    double max_climb_rate = 10.0;   // ограничение вертикальной скорости, м/с
};

// Мультироторные системы добавляют ещё один контракт — число моторов
class MultirotorUAV : public UAV {
public:
    using UAV::UAV;
    virtual int motor_count() const = 0;
};

class Quadcopter : public MultirotorUAV {
public:
    explicit Quadcopter(double battery = 86.0, double altitude = 150.0);
    void takeoff() override;
    void land() override;
    QString type() const override;
    int motor_count() const override;
};

class Hexacopter : public MultirotorUAV {
public:
    explicit Hexacopter(double battery = 92.0, double altitude = 0.0);
    void takeoff() override;
    void land() override;
    QString type() const override;
    int motor_count() const override;
};

// Самолётная схема — НЕ мультиротор, motor_count() у неё нет
class FixedWingUAV : public UAV {
public:
    explicit FixedWingUAV(double battery = 74.0, double altitude = 0.0);
    void takeoff() override;
    void land() override;
    QString type() const override;
};

#endif // UAV_H
