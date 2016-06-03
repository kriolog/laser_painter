#include <QApplication>
#include <QLabel>
#include <QCamera>
#include <QCameraImageCapture>

#include "video_frame_grabber.h"
#include "image_modifier.h"
#include "laser_detector.h"
#include "laser_detector_settings.h"
#include "roi_image_widget.h"
#include "track_widget.h"

using namespace laser_painter;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Laser Painter");

    QCamera camera;
    VideoFrameGrabber video_frame_grabber;
    camera.setViewfinder(&video_frame_grabber);
    QCameraImageCapture image_capture(&camera);
    QImageEncoderSettings image_settings;
//     imageSettings.setCodec("image/jpeg");
    image_settings.setResolution(640, 480);
    image_capture.setEncodingSettings(image_settings);

    ROIImageWidget roi_image_wgt;
    QObject::connect(&video_frame_grabber, &VideoFrameGrabber::frameAvailable, &roi_image_wgt, &ROIImageWidget::setImage);

    ImageModifier image_modifier;
    QObject::connect(&roi_image_wgt, &ROIImageWidget::roiChanged, &image_modifier, &ImageModifier::setROI);
    QObject::connect(&video_frame_grabber, &VideoFrameGrabber::frameAvailable, &image_modifier, &ImageModifier::run);

    LaserDetector laser_detector;
    QObject::connect(&image_modifier, &ImageModifier::imageAvailable, &laser_detector, &LaserDetector::run);

    TrackWidget track_widget(100, 1000, image_capture.encodingSettings().resolution());
    QObject::connect(&laser_detector, SIGNAL(laserPosition(const QPointF&, bool)), &track_widget, SLOT(addTip(const QPointF&, bool)));

    camera.start();
    roi_image_wgt.show();
    track_widget.show();

    LaserDetectorSettings laser_detector_settings(laser_detector);
    laser_detector_settings.show();

    return app.exec();
}