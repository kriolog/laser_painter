#include "main_window.h"

#include <QSettings>

#include "video_frame_grabber.h"
#include "camera_settings.h"
#include "image_modifier.h"
#include "laser_detector.h"
#include "point_modifier.h"
#include "laser_detector_settings.h"
#include "roi_image_widget.h"
#include "track_widget.h"

namespace laser_painter {

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
{
    createWidgets();
}

void MainWindow::createWidgets()
{
    VideoFrameGrabber* video_frame_grabber = new VideoFrameGrabber(this);

    _camera_settings = new CameraSettings(video_frame_grabber);

    ROIImageWidget* roi_image_wgt = new ROIImageWidget();
    QObject::connect(video_frame_grabber, &VideoFrameGrabber::frameAvailable, roi_image_wgt, &ROIImageWidget::setImage);

    ImageModifier* image_modifier = new ImageModifier(this);
    QObject::connect(roi_image_wgt, SIGNAL(roiChanged(const QRect&, const QSize&)), image_modifier, SLOT(setROI(const QRect&)));
    QObject::connect(video_frame_grabber, &VideoFrameGrabber::frameAvailable, image_modifier, &ImageModifier::run);
    image_modifier->setScale(0.7); // TODO:replace to connect

    LaserDetector* laser_detector = new LaserDetector(this);
    QObject::connect(image_modifier, &ImageModifier::imageAvailable, laser_detector, &LaserDetector::run);

    PointModifier* point_modifier = new PointModifier(this);
    QObject::connect(roi_image_wgt, SIGNAL(roiChanged(const QRect&, const QSize&)), point_modifier, SLOT(setROI(const QRect&)));
    QObject::connect(laser_detector, &LaserDetector::laserPosition, point_modifier, &PointModifier::run);
    point_modifier->setUnscale(0.7); // TODO:replace to connect

    TrackWidget* track_widget = new TrackWidget(100, 1000);
    track_widget->setCanvasSize(QSize(640, 480));// image_capture->encodingSettings().resolution()); // TODO:replace to connect
    QObject::connect(point_modifier, SIGNAL(pointAvailable(const QPointF&, bool)), track_widget, SLOT(addTip(const QPointF&, bool)));

    _laser_detector_settings = new LaserDetectorSettings(laser_detector);

    setCentralWidget(roi_image_wgt);
    // TODO: set parents for track_widget and laser_detector_settings _camera_settings
    track_widget->show();
    _camera_settings->show();
    _laser_detector_settings->show();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    _camera_settings->writeSettings();
    _laser_detector_settings->writeSettings();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

} // namespace laser_painter