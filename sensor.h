#ifndef SENSOR_H
#define SENSOR_H

#include <QString>

// ===== Модуль «Сенсоры» =====
// Абстрактный класс с частичной реализацией: read() у каждого прибора свой.
class Sensor {
public:
    explicit Sensor(const QString& sensor_name);
    virtual ~Sensor() = default;

    virtual double read() = 0;
    virtual bool is_ready() const { return ready; }
    virtual QString name() const { return sensor_name; }

protected:
    QString sensor_name;   // общие поля иерархии — protected
    bool ready = true;
};

class Gyroscope : public Sensor {
public:
    explicit Gyroscope(double initial_value);
    double read() override;
    void set_ready(bool value);     // сеттер — легальная дверь в капсулу (имитация отказа)

private:
    double angular_velocity;        // рад/с
};

class Accelerometer : public Sensor {
public:
    explicit Accelerometer(double initial_value);
    double read() override;

private:
    double acceleration;            // м/с²
};

class Barometer : public Sensor {
public:
    explicit Barometer(double initial_value);
    double read() override;

private:
    double pressure;                // гПа
};

#endif // SENSOR_H
