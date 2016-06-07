#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QDockWidget;

namespace laser_painter {
    class ROIImageWidget;
    class LaserDetectorSettings;
    class LaserDetectorCalibrationDialog;
    class CameraSettings;
    class TrackWidget;
    class TrackerSettings;
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
    void createActions();
    void createMenus();
    void createWidgets();

    void writeSettings();

private slots:
    void updateStreamsVisibility(QAction* stream_act);
    void toggleFullScreen(bool enable = false);

private:
    QAction* _exit_act;
    QAction* _viwe_act;
    QAction* _camera_capture_act;
    QAction* _laser_tracker_act;
    QAction* _camera_tracker_act;
    QAction* _full_screen_act;
    QMenu* _file_mu;
    QMenu* _view_mu;
    ROIImageWidget* _roi_image_wgt;
    LaserDetectorSettings* _laser_detector_settings;
    TrackerSettings* _tracker_settings;
    LaserDetectorCalibrationDialog* _laser_detector_calibration_dialog;
    CameraSettings* _camera_settings;
    TrackWidget* _track_widget;
    QDockWidget* _settings_dk;
};

} // namespace laser_painter

#endif // MAIN_WINDOW_H
