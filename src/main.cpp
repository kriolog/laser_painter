#include <QApplication>
#include <QLabel>
#include <QCamera>
#include <QCameraImageCapture>

#include "video_frame_grabber.h"
#include "laser_detector.h"
#include "image_widget.h"
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

    ImageWidget image_wgt;
    QObject::connect(&video_frame_grabber, &VideoFrameGrabber::frameAvailable, &image_wgt, &ImageWidget::setImage);

    LaserDetector laser_detector(
        160, // hue_min
        20, // hue_max
        0, // saturation_min
        50, // saturation_max
        200, // value_min
        255, // value_max
        5 // blob_closing_size
    );
    TrackWidget track_widget(100, 1000, image_capture.encodingSettings().resolution());
    QObject::connect(&video_frame_grabber, &VideoFrameGrabber::frameAvailable, &laser_detector, &LaserDetector::run);
    QObject::connect(&laser_detector, SIGNAL(laserPosition(const QPointF&, bool)), &track_widget, SLOT(addTip(const QPointF&, bool)));

    camera.start();
    image_wgt.show();
    track_widget.show();

    return app.exec();
}