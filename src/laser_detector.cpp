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
    QObject* parent
) :
    QObject(parent)
{
    setHueRange(hue_min, hue_max);
    setSaturationParams(saturation_min, saturation_max);
    setValueRange(value_min, value_max);
    setBlobClosingSize(blob_closing_size);
    setAreaFilterParams(with_area_filter, min_blob_area, max_blob_area);
    setCircularityFilterParams(with_circularity_filter, min_blob_circularity, max_blob_circularity);
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
    if(_hue_min < _hue_max) {
        thresholdInRnage(*h, *h, _hue_min, _hue_max);
    } else if (_hue_min > _hue_max + 1) {
        /// [0, max] or [min, 180), max <= min < 180
        thresholdInRnage(*h, *h, _hue_max + 1, _hue_min - 1);
        cv::bitwise_not(*h, *h);
    } // else(_hue_min == _hue_max || _hue_min == _hue_max + 1) entire hue range should be kept

    // Saturation
    thresholdInRnage(*s, *s, _saturation_min, _saturation_max);

    // Value
    thresholdInRnage(*v, *v, _value_min, _value_max);

    // Detected blob(s)
    cv::Mat* blob = h;
    cv::bitwise_and(*blob, *s, *blob);
    cv::bitwise_and(*blob, *v, *blob);

    // Close detected blobs
    cv::morphologyEx(*blob, *blob, cv::MORPH_CLOSE,
        cv::getStructuringElement(cv::MORPH_ELLIPSE,
            cv::Size(_blob_closing_size, _blob_closing_size)));

    // Detect blob contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(*blob, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    bool with_filters = _with_area_filter || _with_circularity_filter;

    // No contours detected
    if(contours.size() == 0) {
        emit laserPosition(QPointF(), false);
        return;
    }

    // More than one contour detected and no postprocessing filtering
    if(contours.size() != 1 && !with_filters) {
        emit laserPosition(QPointF(), false);
        return;
    }

    // Index of the laser blob contour. Zero if no filtering, otherwise
    // the position of the blob contour after filtering or -1 if more than one
    // contour passed filtering.
    int laser_blob_contour_id = 0;

    if(with_filters) {
        // No contours found
        laser_blob_contour_id = -1;
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
                if(perimeter = 0)
                    continue;
                double circularity = 4 * CV_PI * area / (perimeter * perimeter);
                if(circularity < _blob_circularity_min || circularity  > _blob_circularity_max)
                    continue;
            }

            // Contour passed filters
            if(laser_blob_contour_id < 0) {
                // First contour, save its id
                laser_blob_contour_id = i;
            } else {
                // Second contour, filtering failed: set id to -1 and break.
                laser_blob_contour_id = -1;
                break;
            }
        }
    }

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

    emit(QPointF(moments.m10 / moments.m00, moments.m10 / moments.m00));
}

void LaserDetector::setHueRange(uchar min, uchar max)
{
    Q_ASSERT(min < 180 && max < 180);

    _hue_min = min;
    _hue_max = max;
}

void LaserDetector::setSaturationParams(uchar min, uchar max)
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

void LaserDetector::setAreaFilterParams(bool enable, uint min, uint max)
{
    Q_ASSERT(min <= max);

    _with_circularity_filter = enable;
    _blob_circularity_min = min;
    _blob_circularity_max = max;
}

void LaserDetector::setCircularityFilterParams(bool enable, uint min, uint max)
{
    Q_ASSERT(min <= max);

    _with_area_filter = enable;
    _blob_area_min = min;
    _blob_area_max = max;
}

void LaserDetector::setBlobClosingSize(uint size)
{
    _blob_closing_size = size;
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

void LaserDetector::thresholdInRnage(const cv::Mat& src, cv::Mat& dst, uchar min, uchar max) const
{
    Q_ASSERT(src.type() == CV_8U && dst.type() == CV_8U);

    cv::inRange(src, cv::Scalar(min), cv::Scalar(max), dst);
}

} // namespace laser_painter