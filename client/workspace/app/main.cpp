#include "mainwindow.h"
#include "ScheduleData.hpp"
#include <QApplication>
#include <string>
#include <QString>

int main(int argc, char *argv[])
{   
    // Example class from ScheduleData.hpp
    TimeBlock exampleBlock{
        "block1",
        "Monday",
        800,
        945,
        105
    };
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
