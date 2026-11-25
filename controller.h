#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QString>

// ===== Модуль «Контроллер» =====
class FlightController {
public:
    virtual ~FlightController() = default;
    virtual double calculate_control(double error, double dt) = 0;
    virtual QString controller_name() const = 0;
};

class PIDController : public FlightController {
public:
    PIDController(double kp, double ki, double kd);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;

private:
    double Kp, Ki, Kd;
    double integral = 0.0;          // накопленное состояние — private
    double previous_error = 0.0;
    bool first_cycle = true;
    double integral_limit = 20.0;   // ограничение интеграла (защита от «раскачки»)
};

class SimpleController : public FlightController {
public:
    explicit SimpleController(double coefficient);
    double calculate_control(double error, double dt) override;
    QString controller_name() const override;

private:
    double coefficient;
};

#endif // CONTROLLER_H
