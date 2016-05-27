#ifndef CAMERA_FRAME_GRABBER_H
#define CAMERA_FRAME_GRABBER_H

#include <QAbstractVideoSurface>

// TMP:
class QPixmap;

namespace laser_painter {

class CameraFrameGrabber : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit CameraFrameGrabber(QObject* parent = 0);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

    bool present(const QVideoFrame& frame);

signals:
    void frameAvailable(const QPixmap& frame);
};

} // laser_painter

#endif // CAMERA_FRAME_GRABBER_H