#include "video_frame_grabber.h"

#include <QCamera>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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

    QVideoFrame::PixelFormat frame_pixel_format = frame_shallow_copy.pixelFormat();
    QImage frame_image;
    if(
        frame_pixel_format == QVideoFrame::Format_YUV444 ||
        frame_pixel_format == QVideoFrame::Format_YUV420P ||
        frame_pixel_format == QVideoFrame::Format_YV12 ||
        frame_pixel_format == QVideoFrame::Format_UYVY ||
        frame_pixel_format == QVideoFrame::Format_YUYV
    )
        frame_image = YUVQVideoFrame2QImage(frame_shallow_copy);
    else
        frame_image = QImage(
            frame_shallow_copy.bits(),
            frame_shallow_copy.width(),
            frame_shallow_copy.height(),
            frame_shallow_copy.bytesPerLine(),
            // Warning: QImage::Format_Invalid will be returned for unsupported.
            QVideoFrame::imageFormatFromPixelFormat(frame_shallow_copy.pixelFormat())
        );
    emit frameAvailable(frame_image);

    // Unmap from CPU
    frame_shallow_copy.unmap();
    return true;
}

void VideoFrameGrabber::installCamera(QCamera* camera)
{
    Q_ASSERT(camera);
    if(camera->status() == QCamera::ActiveStatus)
        camera->stop();
    camera->setViewfinder(this);
    camera->start();
}

QImage VideoFrameGrabber::YUVQVideoFrame2QImage(const QVideoFrame& frame) const
{
    /*cv::ColorConversionCodes*/ int cv_color_conversion_code;
    switch(frame.pixelFormat()) {
    case QVideoFrame::Format_YUV444:
        cv_color_conversion_code = cv::COLOR_YCrCb2RGB;
        break;
    case QVideoFrame::Format_YUV420P:
        cv_color_conversion_code = cv::COLOR_YUV2RGB_I420 ;
        break;
    case QVideoFrame::Format_YV12:
        cv_color_conversion_code = cv::COLOR_YUV2RGB_YV12;
        break;
    case QVideoFrame::Format_UYVY:
        cv_color_conversion_code = cv::COLOR_YUV2RGB_UYVY;
        break;
    case QVideoFrame::Format_YUYV:
        cv_color_conversion_code = cv::COLOR_YUV2RGB_YUYV;
        break;
    default:
        emit warning("Camera color space format is not supported");
        return QImage();
    }

    cv::Mat yuv_mat(frame.height(), frame.width(), CV_8UC3, (void*) frame.bits(), frame.bytesPerLine());
    cv::Mat rgb_mat;
    cv::cvtColor(yuv_mat, rgb_mat, cv_color_conversion_code);
    return QImage((uchar*) rgb_mat.data, rgb_mat.cols, rgb_mat.rows, rgb_mat.step, QImage::Format_RGB888).copy();
}

} // namespace laser_painter