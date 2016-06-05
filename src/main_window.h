#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

namespace laser_painter {
    class LaserDetectorSettings;
    class CameraSettings;
}

namespace laser_painter {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void createWidgets();
    void writeSettings();

private:
    LaserDetectorSettings* _laser_detector_settings;
    CameraSettings* _camera_settings;
};

} // namespace laser_painter

#endif // MAIN_WINDOW_H
