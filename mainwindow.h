#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <vector>

#include "uav.h"
#include "sensor.h"
#include "navigation.h"
#include "controller.h"
#include "autopilot.h"

class QTimer;
class QGraphicsScene;
class QGraphicsRectItem;
class QGraphicsPathItem;
class QGraphicsLineItem;
class QChart;
class QLineSeries;
class QValueAxis;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Обработчики кнопок оператора (команды — вниз, в модель)
    void start_clicked();
    void pause_clicked();
    void fault_sensor_clicked();
    void fault_gps_clicked();
    void restore_gps_clicked();
    void reset_clicked();
    void target_changed(double value);

    // Приёмники сигнала telemetry_updated: один сигнал — три слота
    void update_labels(double altitude, double error, double control);
    void update_chart(double altitude, double error, double control);
    void update_scene(double altitude, double error, double control);

    // Приёмники аварийных сигналов модели
    void handle_fault(const QString& reason);
    void handle_navigation_changed(const QString& name);

private:
    void setup_model();
    void setup_fleet_table();
    void setup_chart();
    void setup_scene();
    void setup_connections();

    void set_status(const QString& text, const QString& style);
    void log_event(const QString& text);

    Ui::MainWindow* ui;

    // --- Модель ---
    UAV* vehicle = nullptr;               // управляемый аппарат
    std::vector<UAV*> fleet;              // парк для таблицы
    Gyroscope* gyro = nullptr;
    Accelerometer* accelerometer = nullptr;
    Barometer* barometer = nullptr;
    GPSNavigation* gps = nullptr;
    InertialNavigation* inertial = nullptr;
    VisualNavigation* visual = nullptr;
    NavigationManager* navigation = nullptr;
    FlightController* controller = nullptr;
    Autopilot* autopilot = nullptr;

    // --- Представление ---
    QTimer* cycle_timer = nullptr;
    QChart* chart = nullptr;
    QLineSeries* series = nullptr;
    QValueAxis* axis_x = nullptr;
    QValueAxis* axis_y = nullptr;
    double chart_time = 0.0;
    double chart_min = 1e9;
    double chart_max = -1e9;

    QGraphicsScene* scene = nullptr;
    QGraphicsRectItem* marker = nullptr;
    QGraphicsPathItem* trajectory = nullptr;
    QGraphicsLineItem* target_line = nullptr;
    double scene_time = 0.0;

    QElapsedTimer uptime;                 // время работы станции для журнала
    bool fault_dialog_shown = false;      // признак «первой аварии»
};

#endif // MAINWINDOW_H
