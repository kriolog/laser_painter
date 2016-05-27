#include <QApplication>
#include <QLabel>
#include <QCamera>

#include "camera_frame_grabber.h"

using namespace laser_painter;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Laser Painter");

    QCamera camera;
    CameraFrameGrabber camera_frame_grabber;
    camera.setViewfinder(&camera_frame_grabber);

    QLabel label;
    QObject::connect(&camera_frame_grabber, &CameraFrameGrabber::frameAvailable, &label, &QLabel::setPixmap);

    camera.start();
    label.show();

    return app.exec();
}