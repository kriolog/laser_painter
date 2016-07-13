#include "laser_detector.h"

#include <vector>

#include <QImage>
#include <QPointF>
#include <QSize>
#include <QDebug>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace laser_painter {

/// based on "LASER SPOT DETECTION" of Matej MESKO and Stefan TOTH, 2013
LaserDetector::LaserDetector
(
    QObject* parent,
    uchar hue_min,
    uchar hue_max,
    bool with_saturation,
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
    bool emit_filtered_images
) :
    QObject(parent)
{
    setHueRange(hue_min, hue_max);
    setWithSaturation(with_saturation);
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
//     cv::Mat* h = hsv;
//     cv::Mat* s = hsv + 1;
    cv::Mat* v = hsv + 2;

    // Average input value threshold
    static const uchar aiv_thresh = 77;
    uchar aiv = std::round(cv::mean(*v).val[0]);
    if(aiv > aiv_thresh) {
        emit warning(QString(tr("Laser detector: Average input value %1 is more than the threshold value %2. Decrease the camera exposure.")).arg(aiv).arg(aiv_thresh));
        emit laserPosition(QPointF(), false);
        return;
    }

    // Dynamic value threshold
    uchar DV_thresh;
    double min_value, max_value;
    cv::minMaxLoc(*v, &min_value, &max_value);
    DV_thresh = std::round(0.95 * max_value);

    // Filter by the dynamic value threshold
    cv::Mat v_bin = *v >= DV_thresh;
    emit blobsFilteredAvailable(cvMat2QImageBin(v_bin));

    // Break if there's too much bright pixels.
    static const int max_bright_pixels_nb = 100;
    if(cv::countNonZero(v_bin) > max_bright_pixels_nb) {
        emit warning(QString(tr("Too much (%1) bright pixels. Try downscaling the camera image or decreasing the camera exposure.")).arg(cv::countNonZero(v_bin)));
        emit laserPosition(QPointF(), false);
        return;
    }

     // Detect spots and find the laser spot, if any
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(v_bin, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    static const int max_bright_pixels_in_laser_spot_nb = 36;
    bool laser_spot_found = false;
    // Resulting laser position
    QPointF laser_pos;
    int w = v->cols;
    int h = v->rows;
    static const uchar laser_DV_thresh = std::round(0.9 * max_value);
    static const uchar bright_pixels_ratio_thresh = 0.3;
    qDebug() << "contours size: " << contours.size();
    for(size_t i = 0, size = contours.size(); i < size; ++i) {
        cv::Moments moments = cv::moments(contours[i]);
        double area = moments.m00;
        qDebug() << "contour " << i << ": area :" << area;
        if(area < max_bright_pixels_in_laser_spot_nb)
            continue;
        else if(laser_spot_found) {
            emit warning(QString(tr("Two or more laser spots were found.")));
            emit laserPosition(QPointF(), false);
            return;
        }
        int diameter = std::ceil(std::sqrt(area));
        qDebug() << "contour " << i << ": diameter :" << diameter;
        QPointF blob_center_pos = center(moments);
        qDebug() << "contour " << i << ": center :" << blob_center_pos;
        int ci = std::round(blob_center_pos.x());
        int cj = std::round(blob_center_pos.y());
        if(ci - diameter < 0 || cj - diameter < 0 || ci + diameter >= w || cj + diameter >= h)
            // Blob is on the image border, can't compute the
            continue;
        int bright_pixels_nb = 0;
        for(int i = ci - diameter; i <= ci + diameter; ++i)
            for(int j = cj - diameter; j <= cj + diameter; ++j)
                if(v->at<uchar>(i, j) >= laser_DV_thresh)
                    ++bright_pixels_nb;
        if(bright_pixels_nb == 0)
            continue;
        int cropped_area = 2 * diameter + 1;
        cropped_area *= cropped_area;
        Q_ASSERT(cropped_area > 0);
        qDebug() << "contour " << i << ": bright_pixels_ratio: " << bright_pixels_nb << cropped_area << static_cast<double>(bright_pixels_nb) / cropped_area;
        if((static_cast<double>(bright_pixels_nb) / cropped_area) > bright_pixels_ratio_thresh)
            continue;
        laser_spot_found = true;
        laser_pos = blob_center_pos;
    }

    emit laserPosition(laser_pos);
}

void LaserDetector::setHueRange(uchar min, uchar max)
{
    Q_ASSERT(min < 180 && max < 180);

    _hue_min = min;
    _hue_max = max;
}

void LaserDetector::setWithSaturation(bool enabled)
{
    _with_saturation = enabled;
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
        emit warning("Laser detector: the input image format is not supported");
        return cv::Mat();
    }

    // Straightforward conversion for RGB888 images
    if(image.format() == QImage::Format_RGB888)
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