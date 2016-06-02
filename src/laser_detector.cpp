#include "laser_detector.h"

#include <vector>

#include <QImage>
#include <QPointF>
#include <QSize>
#include <QDebug>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace laser_painter {

LaserDetector::LaserDetector
(
    uchar hue_min,
    uchar hue_max,
    uchar saturation_min,
    uchar saturation_max,
    uchar value_min,
    uchar value_max,
    uint blob_closing_size,
    bool with_area_filter,
    uint min_blob_area,
    uint max_blob_area,
    bool with_circularity_filter,
    uint min_blob_circularity,
    uint max_blob_circularity,
    bool emit_filtered_images,
    QObject* parent
) :
    QObject(parent)
{
    setHueRange(hue_min, hue_max);
    setSaturationRange(saturation_min, saturation_max);
    setValueRange(value_min, value_max);
    setBlobClosingSize(blob_closing_size);
    setBlobAreaParams(with_area_filter, min_blob_area, max_blob_area);
    setBlobCircularityParams(with_circularity_filter, min_blob_circularity, max_blob_circularity);
    setEmitFilteredImages(emit_filtered_images);
}

void LaserDetector::run(const QImage& image) const
{
    // Convert to HSV
    cv::Mat hsv_mat;
    cv::cvtColor(QImage2cvMat(image), hsv_mat, cv::COLOR_RGB2HSV);

    if(hsv_mat.empty()) {
        emit laserPosition(QPointF(), false);
        return;
    }

    // Split into channels
    cv::Mat hsv[3];
    cv::split(hsv_mat, hsv);
    cv::Mat* h = hsv;
    cv::Mat* s = hsv + 1;
    cv::Mat* v = hsv + 2;

    // Hue
    if(_hue_min <= _hue_max)
        *h = *h >= _hue_min & *h <= _hue_max;
    else
        *h = *h <= _hue_max | *h >= _hue_min;

    // Saturation
    *s = *s >= _saturation_min & *s <= _saturation_max;

    // Value
    *v = *v >= _value_min & *v <= _value_max;

    // Emit fitered HSV:
    if(_emit_filtered_images) {
        emit hueFilteredAvailable(cvMat2QImageBin(*h));
        emit saturationFilteredAvailable(cvMat2QImageBin(*s));
        emit valueFilteredAvailable(cvMat2QImageBin(*v));
    }

    // Detected blob(s)
    cv::Mat blob;
    cv::bitwise_and(*h, *s, blob);
    cv::bitwise_and(blob, *v, blob);

    // Close detected blobs
    if(_blob_closing_size > 0)
        cv::morphologyEx(
            blob,
            blob,
            cv::MORPH_CLOSE,
            cv::getStructuringElement(cv::MORPH_ELLIPSE,
                cv::Size(_blob_closing_size, _blob_closing_size))
        );

    // Detect blob contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(blob, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    bool with_filters = _with_area_filter || _with_circularity_filter;

    if(contours.size() == 0 || (contours.size() != 1 && !with_filters)) {
        // No contours detected OR
        // more than one contour detected and no postprocessing filtering.
        // Detection failed.
        emit laserPosition(QPointF(), false);
        // Emit fitered blob(s):
        if(_emit_filtered_images)
            emit blobsFilteredAvailable(cvMat2QImageBin(blob));
        return;
    } else if(contours.size() == 1 && !with_filters) {
        // exactly one contour detected and no postprocessing filtering
        // Laser blob detected
        cv::Moments moments = cv::moments(contours[0]);
        if(moments.m00 == 0)
            emit laserPosition(QPointF(), false);
        else
            emit laserPosition(center(moments));
        return;
    }
    Q_ASSERT(with_filters);

    // Index of the laser blob contour after filtering or -1 if no contours
    // passed filtering or -2 if more than one contour passed filtering.
    int laser_blob_contour_id = -1;
    if(_emit_filtered_images)
        blob.setTo(0);
    for(size_t i = 0, size = contours.size(); i < size; ++i) {
        cv::Moments moments = cv::moments(contours[i]);
        // Filter by area
        if(_with_area_filter) {
            double area = moments.m00;
            if(area < _blob_area_min || area > _blob_area_max)
                continue;
        }
        // Filter by blob_circularity
        if(_with_circularity_filter) {
            double area = moments.m00;
            double perimeter = cv::arcLength(cv::Mat(contours[i]), true);
            if(perimeter == 0.)
                continue;
            double circularity = 4 * CV_PI * area / (perimeter * perimeter);
            if(circularity < _blob_circularity_min || circularity  > _blob_circularity_max)
                continue;
        }

        // Contour passed filters
        Q_ASSERT(
            laser_blob_contour_id == -1 ||
            laser_blob_contour_id == -2 ||
            laser_blob_contour_id >= 0
        );
        if(laser_blob_contour_id == -1) {
            // First contour, save its id
            laser_blob_contour_id = i;
        } else /*if(laser_blob_contour_id >= 0 || laser_blob_contour_id == -2)*/ {
            // Second contour, filtering failed: set id to -2 and break (if
            // shouldn't emit filtered images).
            laser_blob_contour_id = -2;
            if(!_emit_filtered_images)
                break;
        }

        if(_emit_filtered_images)
            cv::drawContours(blob, contours, i, 255, CV_FILLED);
    }

    // Emit fitered blob(s):
    if(_emit_filtered_images)
        emit blobsFilteredAvailable(cvMat2QImageBin(blob));

    // No pertinent contours detected
    if(laser_blob_contour_id < 0) {
        emit laserPosition(QPointF(), false);
        return;
    }

    // Detect laser blob center
    cv::Moments moments = cv::moments(contours[laser_blob_contour_id]);
    if(moments.m00 == 0) {
        emit laserPosition(QPointF(), false);
        return;
    }

    emit laserPosition(center(moments));
}

void LaserDetector::setHueRange(uchar min, uchar max)
{
    Q_ASSERT(min < 180 && max < 180);

    _hue_min = min;
    _hue_max = max;
}

void LaserDetector::setSaturationRange(uchar min, uchar max)
{
    Q_ASSERT(min <= max);

    _saturation_min = min;
    _saturation_max = max;
}

void LaserDetector::setValueRange(uchar min, uchar max)
{
    Q_ASSERT(min <= max);

    _value_min = min;
    _value_max = max;
}

void LaserDetector::setBlobClosingSize(uint size)
{
    _blob_closing_size = size;
}

void LaserDetector::setBlobAreaParams(bool enable, uint min, uint max)
{
    Q_ASSERT(min <= max);

    _with_area_filter = enable;
    _blob_area_min = min;
    _blob_area_max = max;
}

void LaserDetector::setBlobCircularityParams(bool enable, double min, double max)
{
    Q_ASSERT(min >= 0. && min <= max);

    _with_circularity_filter = enable;
    _blob_circularity_min = min;
    _blob_circularity_max = max;
}

void LaserDetector::setEmitFilteredImages(bool do_emit)
{
    _emit_filtered_images = do_emit;
}

QPointF LaserDetector::center(const cv::Moments& moments) const
{
    Q_ASSERT(moments.m00 > 0);

    return QPointF(moments.m10 / moments.m00, moments.m01 / moments.m00);
}

cv::Mat LaserDetector::QImage2cvMat(const QImage& image) const
{
    if(image.format() == QImage::Format_Invalid) {
        qWarning() << "Laser detector: the input image format is not supported";
        return cv::Mat();
    }

    // Straightforward conversion for RGB888 images
    if(image.format() != QImage::Format_RGB888)
        return cv::Mat(image.height(), image.width(), CV_8UC3, (void*) image.scanLine(0), image.bytesPerLine()).clone();

    // image Should be preconverted in a RGB888 image
    QImage rgb_image = image.convertToFormat(QImage::Format_RGB888);
    return cv::Mat(rgb_image.height(), rgb_image.width(), CV_8UC3, (void*) rgb_image.scanLine(0), rgb_image.bytesPerLine()).clone();
}

QImage LaserDetector::cvMat2QImageBin(const cv::Mat& mat) const
{
    cv::Mat normalized = mat > 0;
    return QImage(normalized.data, normalized.cols, normalized.rows, normalized.step, QImage::Format_Grayscale8).copy();
}

} // namespace laser_painter