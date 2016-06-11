#include "main_window.h"

#include <QSettings>
#include <QAction>
#include <QActionGroup>
#include <QMenuBar>
#include <QShortcut>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QStatusBar>

#include "video_frame_grabber.h"
#include "camera_settings.h"
#include "image_modifier.h"
#include "laser_detector.h"
#include "point_modifier.h"
#include "laser_detector_settings.h"
#include "laser_detector_calibration_dialog.h"
#include "roi_image_widget.h"
#include "track_widget.h"
#include "tracker_settings.h"

namespace laser_painter {

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    QSettings settings;

    setWindowTitle(tr("Laser Tracker"));
    setMinimumSize(1024, 768);
    setFixedSize(settings.value("MainWindow/window_size", minimumSize()).toSize());

    createActions();
    createMenus();
    createWidgets();
}

void MainWindow::createActions()
{

    _exit_act = new QAction(QIcon::fromTheme("application-exit"), tr("E&xit"), this);
    _exit_act->setShortcuts(QKeySequence::Quit);
    _exit_act->setStatusTip(tr("Exit the application"));
    connect(_exit_act,  &QAction::triggered, this, &MainWindow::close);

    // Streams are the camera capture and the lasetr tracker
    QActionGroup* streams_gp = new QActionGroup(this);
    connect(streams_gp, &QActionGroup::triggered, this, &MainWindow::updateStreamsVisibility);
    streams_gp->setExclusive(true);
    _camera_capture_act = new QAction(tr("Camera Capture"), streams_gp);
    _camera_capture_act->setShortcut(QKeySequence(tr("Ctrl+1", "ShowCamera")));
    _camera_capture_act->setCheckable(true);
    _camera_capture_act->setChecked(false);
    _laser_tracker_act = new QAction(tr("Laser Tracker"), streams_gp);
    _laser_tracker_act->setShortcut(QKeySequence(tr("Ctrl+2", "ShowTracker")));
    _laser_tracker_act->setCheckable(true);
    _laser_tracker_act->setChecked(false);
    _camera_tracker_act = new QAction(tr("Both Camera and Tracker"), streams_gp);
    _camera_tracker_act->setShortcut(QKeySequence(tr("Ctrl+3", "ShowCameraAndTracker")));
    _camera_tracker_act->setCheckable(true);
    _camera_tracker_act->setChecked(true);

    _full_screen_act = new QAction(tr("Full Screen Mode"), this);
    _full_screen_act->setCheckable(true);
    _full_screen_act->setShortcut(QKeySequence(tr("Ctrl+F", "ToggleFullScreenExitFullScreen")));
    _exit_act->setStatusTip(tr("Press Esc to exit fulls screen mode"));
    connect(_full_screen_act, &QAction::triggered, this, &MainWindow::toggleFullScreen);
    connect(
        new QShortcut(QKeySequence(tr("Escape", "ExitFullScreen")), this), SIGNAL(activated()),
        this, SLOT(toggleFullScreen())
    );
}

void MainWindow::createMenus()
{
    _file_mu = menuBar()->addMenu(tr("&File"));
    _file_mu->addAction(_exit_act);

    _view_mu = menuBar()->addMenu(tr("&View"));
    _view_mu->addAction(_camera_capture_act);
    _view_mu->addAction(_laser_tracker_act);
    _view_mu->addAction(_camera_tracker_act);
    _view_mu->addSeparator();
    _view_mu->addAction(_full_screen_act);

    // Force actions to work when menu is hidden
    addAction(_camera_capture_act);
    addAction(_laser_tracker_act);
    addAction(_camera_tracker_act);
    addAction(_full_screen_act);
}

void MainWindow::createWidgets()
{
    VideoFrameGrabber* video_frame_grabber = new VideoFrameGrabber(this);

    _camera_settings = new CameraSettings(video_frame_grabber);

    _roi_image_wgt = new ROIImageWidget();
    connect(video_frame_grabber, &VideoFrameGrabber::frameAvailable, _roi_image_wgt, &ROIImageWidget::setImage);

    ImageModifier* image_modifier = new ImageModifier(this);
    connect(_roi_image_wgt, SIGNAL(roiChanged(const QRect&, const QSize&)), image_modifier, SLOT(setROI(const QRect&)));
    connect(video_frame_grabber, &VideoFrameGrabber::frameAvailable, image_modifier, &ImageModifier::run);

    LaserDetector* laser_detector = new LaserDetector(this);
    connect(image_modifier, &ImageModifier::imageAvailable, laser_detector, &LaserDetector::run);

    PointModifier* point_modifier = new PointModifier(this);
    connect(_roi_image_wgt, SIGNAL(roiChanged(const QRect&, const QSize&)), point_modifier, SLOT(setROI(const QRect&)));
    connect(laser_detector, &LaserDetector::laserPosition, point_modifier, &PointModifier::run);

    _track_widget = new TrackWidget();
    connect(point_modifier, SIGNAL(pointAvailable(const QPointF&, bool)), _track_widget, SLOT(addTip(const QPointF&, bool)));
    connect(_camera_settings, &CameraSettings::resolutionChanged, _track_widget, &TrackWidget::setCanvasSize);
    _track_widget->setCanvasSize(_camera_settings->currentResolution());

    _laser_detector_calibration_dialog = new LaserDetectorCalibrationDialog(laser_detector, this);

    _laser_detector_settings = new LaserDetectorSettings(_laser_detector_calibration_dialog);
    connect(_laser_detector_settings, &LaserDetectorSettings::scaleChanged, image_modifier, &ImageModifier::setScale);
    connect(_laser_detector_settings, &LaserDetectorSettings::scaleChanged, point_modifier, &PointModifier::setUnscale);
    _laser_detector_settings->emitScaleChanged();

    _tracker_settings = new TrackerSettings(_track_widget);

    QHBoxLayout* central_widget_lo = new QHBoxLayout();
    central_widget_lo->setMargin(0);
    central_widget_lo->setSpacing(0);
    central_widget_lo->addWidget(_roi_image_wgt);
    central_widget_lo->addWidget(_track_widget);
    QWidget* central_wgt = new QWidget();
    central_wgt->setLayout(central_widget_lo);
    setCentralWidget(central_wgt);

    // Put settings to a dockable widget
    QWidget* settings_wgt = new QWidget();
    QVBoxLayout* settings_lo = new QVBoxLayout();
    settings_wgt->setLayout(settings_lo);
    settings_lo->addWidget(_camera_settings);
    settings_lo->addWidget(_laser_detector_settings);
    settings_lo->addWidget(_tracker_settings);
    settings_lo->addStretch();
    _settings_dk = new QDockWidget(tr("Settings"), this);
    _settings_dk->setFeatures(QDockWidget::DockWidgetMovable);
    _settings_dk->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _settings_dk->setWidget(settings_wgt);
    addDockWidget(Qt::LeftDockWidgetArea, _settings_dk);

    setStatusBar(new QStatusBar());
    connect(video_frame_grabber, &VideoFrameGrabber::warning, this, &MainWindow::showWarning);
    connect(laser_detector, &LaserDetector::warning, this, &MainWindow::showWarning);
}

void MainWindow::updateStreamsVisibility(QAction* stream_act)
{
    if(stream_act == _camera_capture_act) {
        _roi_image_wgt->setVisible(true);
        _track_widget->setVisible(false);
    } else if (stream_act == _laser_tracker_act) {
        _roi_image_wgt->setVisible(false);
        _track_widget->setVisible(true);
    } else if (stream_act == _camera_tracker_act) {
        _roi_image_wgt->setVisible(true);
        _track_widget->setVisible(true);
    } else {
        Q_ASSERT(false);
    }
}

void MainWindow::toggleFullScreen(bool enable)
{
    if(enable) {
        menuBar()->hide();
        _settings_dk->hide();
        setWindowState(Qt::WindowFullScreen);
        if(_full_screen_act->isChecked())
            _full_screen_act->setChecked(true);
    } else {
        menuBar()->show();
        _settings_dk->show();
        setWindowState(Qt::WindowNoState);
        if(_full_screen_act->isChecked())
            _full_screen_act->setChecked(false);
    }
}

void MainWindow::showWarning(const QString& text)
{
    Q_ASSERT(statusBar());
    statusBar()->setStyleSheet("QStatusBar{padding-left:8px;;color:red;}");
    statusBar()->showMessage(text, 5000);
}

void MainWindow::writeSettings()
{
    QSettings settings;

    _laser_detector_settings->writeSettings();
    _laser_detector_calibration_dialog->writeSettings();
    _tracker_settings->writeSettings();

    settings.beginGroup("MainWindow");

    settings.setValue("window_size", size());

    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
    _laser_detector_calibration_dialog->deleteLater();
}

} // namespace laser_painter