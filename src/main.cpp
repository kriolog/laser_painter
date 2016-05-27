#include <QApplication>
#include <QLabel>
#include <QCamera>

#include "video_frame_grabber.h"
#include "image_widget.h"

using namespace laser_painter;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Laser Painter");

    QCamera camera;
    VideoFrameGrabber video_frame_grabber;
    camera.setViewfinder(&video_frame_grabber);

    ImageWidget image_wgt;
    QObject::connect(&video_frame_grabber, &VideoFrameGrabber::frameAvailable, &image_wgt, &ImageWidget::setFrame);

    camera.start();
    image_wgt.show();

    return app.exec();
}