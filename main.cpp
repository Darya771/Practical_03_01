#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);   // инфраструктура: очередь и диспетчеризация событий
    MainWindow window;
    window.show();
    return app.exec();              // запуск цикла событий
}
