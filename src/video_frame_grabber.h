#ifndef VIDEO_FRAME_GRABBER_H
#define VIDEO_FRAME_GRABBER_H

#include <QAbstractVideoSurface>

class QImage;
class QCamera;

namespace laser_painter {

/// Grab frames from cameras or decoded media and send the frame in form
/// of a QImage class.
class VideoFrameGrabber : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    explicit VideoFrameGrabber(QObject* parent = 0);

    /// Define supported formats.
    /// Basically, a format is supported if it's suported by a QImage class.
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

    /// Transform the current frame @param frame into a QImage and emit a signal
    /// of its availability.
    bool present(const QVideoFrame& frame);

public slots:
    void installCamera(QCamera* camera);

signals:
    /// Emit a new available frame image @param frame.
    void frameAvailable(const QImage& frame);

    void warning(const QString& text) const;

private:
    // Convert frames with YUV color schemes with opencv
    // Supported formats:
    // Format_YUV444
    // Format_YUV420P
    // Format_YV12
    // Format_UYVY
    // Format_YUYV
    QImage YUVQVideoFrame2QImage(const QVideoFrame& frame) const;
};

} // namespace laser_painter

#endif // VIDEO_FRAME_GRABBER_H