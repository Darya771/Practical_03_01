#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsPathItem>
#include <QGraphicsLineItem>
#include <QPainterPath>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QPainter>
#include <algorithm>
#include <cmath>

// Пересчёт «полётных» координат в координаты сцены
static double scene_y(double altitude) { return 150.0 - altitude * 0.6; }

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow) {
    ui->setupUi(this);
    uptime.start();

    setup_model();
    setup_fleet_table();
    setup_chart();
    setup_scene();
    setup_connections();

    log_event("Станция запущена");
    set_status("Stopped", "color: gray; font-weight: bold;");
    navigation->update();      // первичный выбор навигации (GPS)
}

MainWindow::~MainWindow() {
    cycle_timer->stop();
    delete ui;
    // Объекты модели без родителя-QObject удаляем вручную
    for (UAV* uav : fleet) delete uav;       // vehicle входит в fleet
    delete gyro;
    delete accelerometer;
    delete barometer;
    delete gps;
    delete inertial;
    delete visual;
    delete controller;
    // autopilot, navigation, cycle_timer, scene — дети окна, удалятся сами
}

// ======================= Сборка модели =======================
void MainWindow::setup_model() {
    vehicle = new Quadcopter();               // старт на высоте 150 м
    fleet = { vehicle, new FixedWingUAV(), new Hexacopter() };

    gyro = new Gyroscope(1.2);
    accelerometer = new Accelerometer(9.81);
    barometer = new Barometer(1012.5);
    std::vector<Sensor*> sensors = { gyro, accelerometer, barometer };

    gps = new GPSNavigation(vehicle);
    inertial = new InertialNavigation(vehicle);
    visual = new VisualNavigation(vehicle);
    navigation = new NavigationManager(gps, inertial, visual, this);

    controller = new PIDController(0.5, 0.1, 0.05);

    autopilot = new Autopilot(vehicle, sensors, navigation, controller, this);
    autopilot->set_target_altitude(ui->spinTarget->value());
}

// ======================= Таблица парка (задание 2, вариант 1) =======================
void MainWindow::setup_fleet_table() {
    ui->tableFleet->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableFleet->setEditTriggers(QAbstractItemView::NoEditTriggers);

    for (UAV* uav : fleet) {
        int row = ui->tableFleet->rowCount();
        ui->tableFleet->insertRow(row);
        ui->tableFleet->setItem(row, 0, new QTableWidgetItem(uav->type()));   // полиморфный вызов
        ui->tableFleet->setItem(row, 1, new QTableWidgetItem(
                                    QString::number(uav->battery(), 'f', 0) + " %"));
        ui->tableFleet->setItem(row, 2, new QTableWidgetItem("Connected"));

        // motor_count() есть только у мультироторов — проверяем через dynamic_cast
        QString motors = "—";
        if (auto multirotor = dynamic_cast<const MultirotorUAV*>(uav)) {
            motors = QString::number(multirotor->motor_count());
        }
        ui->tableFleet->setItem(row, 3, new QTableWidgetItem(motors));
    }
}

// ======================= График высоты =======================
void MainWindow::setup_chart() {
    series = new QLineSeries();
    series->setName("Altitude");

    chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Altitude, m");
    chart->legend()->hide();

    axis_x = new QValueAxis();
    axis_x->setTitleText("t, s");
    axis_x->setRange(0, 30);
    axis_y = new QValueAxis();
    axis_y->setRange(100, 250);
    chart->addAxis(axis_x, Qt::AlignBottom);
    chart->addAxis(axis_y, Qt::AlignLeft);
    series->attachAxis(axis_x);
    series->attachAxis(axis_y);

    ui->chartView->setChart(chart);          // владение графиком переходит виджету
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}

// ======================= Сцена с траекторией =======================
void MainWindow::setup_scene() {
    scene = new QGraphicsScene(this);
    scene->setSceneRect(-200, -200, 400, 400);   // иначе сцена «прыгает»
    ui->graphicsView->setScene(scene);

    scene->addLine(-200, scene_y(0), 200, scene_y(0), QPen(Qt::darkGreen, 2));   // земля

    target_line = scene->addLine(-200, scene_y(autopilot->target_altitude()),
                                 200, scene_y(autopilot->target_altitude()),
                                 QPen(Qt::red, 1, Qt::DashLine));

    trajectory = scene->addPath(QPainterPath(), QPen(Qt::blue, 1));
    marker = scene->addRect(-10, -10, 20, 20, QPen(Qt::darkBlue), QBrush(Qt::blue));
    marker->setPos(-190, scene_y(vehicle->altitude()));
}

// ======================= Соединения сигнал–слот =======================
void MainWindow::setup_connections() {
    cycle_timer = new QTimer(this);
    cycle_timer->setInterval(100);   // 10 Гц

    // Таймер: сначала менеджер навигации проверяет приоритет, затем цикл автопилота
    connect(cycle_timer, &QTimer::timeout, navigation, &NavigationManager::update);
    connect(cycle_timer, &QTimer::timeout, autopilot, &Autopilot::run_cycle);

    // Один сигнал — три приёмника
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::update_labels);
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::update_chart);
    connect(autopilot, &Autopilot::telemetry_updated, this, &MainWindow::update_scene);

    connect(autopilot, &Autopilot::fault_detected, this, &MainWindow::handle_fault);
    connect(navigation, &NavigationManager::active_system_changed,
            this, &MainWindow::handle_navigation_changed);

    // Кнопки и спинбокс
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::start_clicked);
    connect(ui->btnPause, &QPushButton::clicked, this, &MainWindow::pause_clicked);
    connect(ui->btnFaultSensor, &QPushButton::clicked, this, &MainWindow::fault_sensor_clicked);
    connect(ui->btnFaultGps, &QPushButton::clicked, this, &MainWindow::fault_gps_clicked);
    connect(ui->btnRestoreGps, &QPushButton::clicked, this, &MainWindow::restore_gps_clicked);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::reset_clicked);
    connect(ui->spinTarget, &QDoubleSpinBox::valueChanged, this, &MainWindow::target_changed);
}

// ======================= Кнопки оператора =======================
void MainWindow::start_clicked() {
    cycle_timer->start();
    set_status("Running", "color: green; font-weight: bold;");
    log_event("Старт цикла автопилота");
}

void MainWindow::pause_clicked() {
    cycle_timer->stop();                      // модель не пересоздаётся — состояние сохраняется
    set_status("Paused", "color: #d08000; font-weight: bold;");
    log_event("Пауза");
}

void MainWindow::fault_sensor_clicked() {
    gyro->set_ready(false);                   // следующий цикл обнаружит отказ
    log_event("Оператор: имитация отказа гироскопа");
}

void MainWindow::fault_gps_clicked() {
    gps->set_valid(false);
    log_event("Оператор: имитация потери GPS");
    navigation->update();                     // сразу показать переключение
}

void MainWindow::restore_gps_clicked() {
    gps->set_valid(true);
    log_event("Оператор: GPS восстановлен");
    navigation->update();
}

void MainWindow::reset_clicked() {
    gyro->set_ready(true);                    // восстановление через тот же сеттер
    fault_dialog_shown = false;               // следующая авария снова покажет диалог
    set_status("Mission status: Active", "color: green; font-weight: bold;");
    log_event("Сброс: работа возобновлена");
    cycle_timer->start();
}

void MainWindow::target_changed(double value) {
    autopilot->set_target_altitude(value);    // команда — вниз, через публичный метод
    target_line->setLine(-200, scene_y(value), 200, scene_y(value));
    log_event(QString("Новая целевая высота: %1 м").arg(value, 0, 'f', 0));
}

// ======================= Приёмники телеметрии =======================
void MainWindow::update_labels(double altitude, double error, double control) {
    ui->labelAltitude->setText(QString::number(altitude, 'f', 1) + " m");
    ui->labelError->setText(QString::number(error, 'f', 1) + " m");
    ui->labelOutput->setText(QString::number(control, 'f', 2));
}

void MainWindow::update_chart(double altitude, double /*error*/, double /*control*/) {
    series->append(chart_time, altitude);
    if (series->count() > 600) {
        series->remove(0);                    // храним последние 60 секунд
    }

    chart_min = std::min(chart_min, altitude);
    chart_max = std::max(chart_max, altitude);
    double target = autopilot->target_altitude();
    axis_y->setRange(std::min(chart_min, target) - 10, std::max(chart_max, target) + 10);
    axis_x->setRange(std::max(0.0, chart_time - 30.0), std::max(30.0, chart_time));

    chart_time += 0.1;
}

void MainWindow::update_scene(double altitude, double /*error*/, double /*control*/) {
    double x = -190.0 + std::fmod(scene_time * 10.0, 380.0);
    double y = scene_y(altitude);

    QPainterPath path = trajectory->path();
    if (path.isEmpty() || x < path.currentPosition().x()) {
        path = QPainterPath(QPointF(x, y));    // новый «круг» по экрану — начинаем след заново
    } else {
        path.lineTo(x, y);                     // след удлиняется, а не пересоздаётся
    }
    trajectory->setPath(path);

    marker->setPos(x, y);                      // маркер перемещается, а не пересоздаётся
    scene_time += 0.1;
}

// ======================= Аварии: три уровня информирования =======================
void MainWindow::handle_fault(const QString& reason) {
    cycle_timer->stop();

    // Уровень 1 — постоянный статус
    set_status("FAULT: " + reason, "color: red; font-weight: bold;");
    // Уровень 2 — журнал (каждая авария)
    log_event("FAULT: " + reason);
    // Уровень 3 — диалог (только первая авария до «Сброса»)
    if (!fault_dialog_shown) {
        fault_dialog_shown = true;
        QMessageBox::warning(this, "Авария",
                             "Причина: " + reason + "\nЦикл автопилота остановлен.");
    }
}

void MainWindow::handle_navigation_changed(const QString& name) {
    ui->labelNav->setText("Active system: " + name);
    if (name == "GPS") {
        ui->labelNav->setStyleSheet("background: #c8f0c8; padding: 4px;");
    } else if (name == "none") {
        ui->labelNav->setStyleSheet("background: #f4b4b4; padding: 4px;");
    } else {
        ui->labelNav->setStyleSheet("background: #ffe066; padding: 4px;");   // резерв — жёлтый
    }
    log_event("Навигация: " + name);
}

// ======================= Вспомогательное (только отображение) =======================
void MainWindow::set_status(const QString& text, const QString& style) {
    ui->labelStatus->setText(text);
    ui->labelStatus->setStyleSheet(style);
}

void MainWindow::log_event(const QString& text) {
    double seconds = uptime.elapsed() / 1000.0;
    ui->listLog->addItem(QString("[%1 s] %2").arg(seconds, 0, 'f', 1).arg(text));
    ui->listLog->scrollToBottom();
}
