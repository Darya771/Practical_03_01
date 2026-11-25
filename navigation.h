#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <QObject>
#include <QString>

class UAV;

struct Position {
    double latitude;
    double longitude;
    double altitude;
};

// ===== Модуль «Навигация» =====
class NavigationSystem {
public:
    virtual ~NavigationSystem() = default;
    virtual Position get_position() const = 0;
    virtual bool is_valid() const = 0;
    virtual QString system_name() const = 0;
};

// GPS: валиден, если спутников >= 4 и нет отказа
class GPSNavigation : public NavigationSystem {
public:
    explicit GPSNavigation(const UAV* vehicle);
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;

    void set_valid(bool value);         // имитация потери GPS
    void set_satellites(int count);

private:
    const UAV* vehicle;                 // откуда «измеряем» высоту
    int satellites = 10;
    bool valid = true;
};

// Инерциальная навигация: доступна всегда, но с дрейфом
class InertialNavigation : public NavigationSystem {
public:
    explicit InertialNavigation(const UAV* vehicle);
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;

private:
    const UAV* vehicle;
    double drift = 0.3;                 // накопленная погрешность, м
};

// Визуальная навигация: валидна, пока цель отслеживается
class VisualNavigation : public NavigationSystem {
public:
    explicit VisualNavigation(const UAV* vehicle);
    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;

    void set_target_tracked(bool value);

private:
    const UAV* vehicle;
    bool target_tracked = true;
};

// Менеджер: сам является NavigationSystem, поэтому автопилот работает
// с ним через тот же интерфейс и не знает о переключениях.
// Приоритет: GPS -> INS -> Visual.
class NavigationManager : public QObject, public NavigationSystem {
    Q_OBJECT
public:
    NavigationManager(NavigationSystem* gps,
                      NavigationSystem* inertial,
                      NavigationSystem* visual,
                      QObject* parent = nullptr);

    Position get_position() const override;
    bool is_valid() const override;
    QString system_name() const override;

public slots:
    void update();      // проверка приоритета и выбор активной системы

signals:
    void active_system_changed(const QString& name);

private:
    NavigationSystem* gps;
    NavigationSystem* inertial;
    NavigationSystem* visual;
    NavigationSystem* active_system = nullptr;   // инкапсуляция: поле закрыто
};

#endif // NAVIGATION_H
