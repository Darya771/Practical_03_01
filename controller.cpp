#include "controller.h"

#include <algorithm>

// ---------- PIDController ----------
PIDController::PIDController(double kp, double ki, double kd)
    : Kp(kp), Ki(ki), Kd(kd) {
}

double PIDController::calculate_control(double error, double dt) {
    integral += error * dt;
    integral = std::clamp(integral, -integral_limit, integral_limit);

    double derivative = 0.0;
    if (!first_cycle && dt > 0.0) {
        derivative = (error - previous_error) / dt;
    }
    first_cycle = false;
    previous_error = error;

    return Kp * error + Ki * integral + Kd * derivative;
}

QString PIDController::controller_name() const { return "PID"; }

// ---------- SimpleController ----------
SimpleController::SimpleController(double coefficient) : coefficient(coefficient) {}

double SimpleController::calculate_control(double error, double /*dt*/) {
    return coefficient * error;
}

QString SimpleController::controller_name() const { return "Simple"; }
