#ifndef VIDEO_FRAME_GRABBER_H
#define VIDEO_FRAME_GRABBER_H

#include <QAbstractVideoSurface>

class QImage;

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

signals:
    /// Emits a new available frame image @param frame.
    void frameAvailable(const QImage& frame);
};

} // namespace laser_painter

#endif // VIDEO_FRAME_GRABBER_H