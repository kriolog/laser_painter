#include "video_frame_grabber.h"

namespace laser_painter {

VideoFrameGrabber::VideoFrameGrabber(QObject *parent) :
    QAbstractVideoSurface(parent) {}

QList<QVideoFrame::PixelFormat> VideoFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    return QList<QVideoFrame::PixelFormat>()
        << QVideoFrame::Format_ARGB32
        << QVideoFrame::Format_ARGB32_Premultiplied
        << QVideoFrame::Format_RGB32
        << QVideoFrame::Format_RGB24
        << QVideoFrame::Format_RGB565
        << QVideoFrame::Format_RGB555
        << QVideoFrame::Format_ARGB8565_Premultiplied
        << QVideoFrame::Format_BGRA32
        << QVideoFrame::Format_BGRA32_Premultiplied
        << QVideoFrame::Format_BGR32
        << QVideoFrame::Format_BGR24
        << QVideoFrame::Format_BGR565
        << QVideoFrame::Format_BGR555
        << QVideoFrame::Format_BGRA5658_Premultiplied
        << QVideoFrame::Format_AYUV444
        << QVideoFrame::Format_AYUV444_Premultiplied
        << QVideoFrame::Format_YUV444
        << QVideoFrame::Format_YUV420P
        << QVideoFrame::Format_YV12
        << QVideoFrame::Format_UYVY
        << QVideoFrame::Format_YUYV
        << QVideoFrame::Format_NV12
        << QVideoFrame::Format_NV21
        << QVideoFrame::Format_IMC1
        << QVideoFrame::Format_IMC2
        << QVideoFrame::Format_IMC3
        << QVideoFrame::Format_IMC4
        << QVideoFrame::Format_Y8
        << QVideoFrame::Format_Y16
        << QVideoFrame::Format_Jpeg
        << QVideoFrame::Format_CameraRaw
        << QVideoFrame::Format_AdobeDng;
}

bool VideoFrameGrabber::present(const QVideoFrame& frame)
{
    if (!frame.isValid()) {
        emit frameAvailable(QImage());
        return false;
    }

    QVideoFrame frame_shallow_copy(frame);
    // Map to CPU
    frame_shallow_copy.map(QAbstractVideoBuffer::ReadOnly);

    // TODO: add support for YUV formats.
    const QImage frame_image(
        frame_shallow_copy.bits(),
        frame_shallow_copy.width(),
        frame_shallow_copy.height(),
        frame_shallow_copy.bytesPerLine(),
        // Warning: QImage::Format_Invalid will be returned for unsupported
        // formats (as YUV formats).
        QVideoFrame::imageFormatFromPixelFormat(frame_shallow_copy.pixelFormat())
    );
    emit frameAvailable(frame_image);

    // Unmap from CPU
    frame_shallow_copy.unmap();
    return true;
}

} // namespace laser_painter