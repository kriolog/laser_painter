#include <QApplication>
#include <QSettings>

#include "main_window.h"

using namespace laser_painter;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Webcast Norge");
    QCoreApplication::setApplicationName("Laser Painter");

    MainWindow mw;
    mw.setWindowTitle("Laser Painter");
    mw.show();
    return app.exec();
}