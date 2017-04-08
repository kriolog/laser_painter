#include "laser_detector.h"

#include <vector>

#include <QImage>
#include <QPointF>
#include <QSize>

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

namespace laser_painter {

/// inspired by "LASER SPOT DETECTION" of Matej MESKO and Stefan TOTH, 2013
LaserDetector::LaserDetector
(
    QObject* parent,

    uchar highest_brightness_min,
    double relative_brightness_min,
    uint blob_closing_size,
    uint nb_blobs_max,
    uint blob_perimeter_min,
    uint blob_perimeter_max,
    int blob_crown_margin_inf,
    int blob_crown_margin_sup,
    uchar hue_min,
    uchar hue_max,
    double blob_crown_valid_pixels_part_min,

    bool emit_filtered_images
) :
    QObject(parent)
{
    setHighestBrightnessMin(highest_brightness_min);
    setRelativeBrightnessMin(relative_brightness_min);
    setBlobClosingSize(blob_closing_size);
    setNbBlobsMax(nb_blobs_max);
    setBlobCrownMargins(blob_crown_margin_inf, blob_crown_margin_sup);
    setBlobPerimeterRange(blob_perimeter_min, blob_perimeter_max);
    setHueRange(hue_min, hue_max);
    setBlobCrownValidPixelsPartMin(blob_crown_valid_pixels_part_min);

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

    // Split input into [hue, saturation, value] channels
    cv::Mat hsv[3];
    cv::split(hsv_mat, hsv);
    cv::Mat* h = hsv;
//     cv::Mat* s = hsv + 1;
    cv::Mat* v = hsv + 2;

    // Dynamic value (brightness) threshold
    double min_brightness, max_brightness;
    cv::minMaxLoc(*v, &min_brightness, &max_brightness);
    if(max_brightness < _highest_brightness_min) {
        // Spots aren't bright enough
        emit laserPosition(QPointF(), false);
        if(_emit_filtered_images)
            emit blobsAvailable(cvMat2QImage(cv::Mat(hsv_mat.size(), CV_8UC1, cv::Scalar(0))));
        return;
    }
    uchar DV_thresh = std::round(_relative_brightness_min * max_brightness);
    // Filter by the dynamic value threshold
    cv::Mat v_bin = *v >= DV_thresh;

    // Morphological closing of the value channel
    if(_blob_closing_size > 0)
        cv::morphologyEx(
            v_bin,
            v_bin,
            cv::MORPH_CLOSE,
            cv::getStructuringElement(cv::MORPH_ELLIPSE,
                cv::Size(_blob_closing_size, _blob_closing_size))
        );

    // Send thresolded blobs image
    if(_emit_filtered_images)
        emit blobsAvailable(cvMat2QImage(v_bin));

    // Detect blobs
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(v_bin, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    // Break if there's too much blobs.
    if(contours.size() > _nb_blobs_max) {
        emit laserPosition(QPointF(), false);
        return;
    }

    // Process blobs
    const cv::Mat cross = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    for(size_t i = 0, size = contours.size(); i < size; ++i) {
        std::vector<cv::Point>& contour = contours[i];

        // Skip blobs with perimeters which too small or too large perimeters
        double P = cv::arcLength(contour, false);
        if(P < _blob_perimeter_min || P > _blob_perimeter_max)
            continue;

        // Blob bounding rect
        cv::Rect blob_rect = cv::boundingRect(contour);
        // Enlarge blob rect for further processing of its crown
        blob_rect.x = std::max<int>(0, blob_rect.x - _blob_crown_margin_sup);
        blob_rect.y = std::max<int>(0, blob_rect.y - _blob_crown_margin_sup);
        blob_rect.width = std::min<int>(v->cols - blob_rect.x, blob_rect.width + 2 * _blob_crown_margin_sup);
        blob_rect.height = std::min<int>(v->rows - blob_rect.y, blob_rect.height + 2 * _blob_crown_margin_sup);

        // Blob subimage
        cv::Mat blob(blob_rect.size(), CV_8UC1, cv::Scalar(0));
        cv::drawContours(blob, contours, i, cv::Scalar(255), CV_FILLED, 4, cv::noArray(), 0, -blob_rect.tl());

        // Blob crown subimage
        cv::Mat blob_crown;
        {
            cv::Mat blob_dilated_inf;
            if(_blob_crown_margin_inf == 0)
                blob_dilated_inf = blob.clone();
            else
                cv::dilate(blob, blob_dilated_inf, cross, cv::Point(-1,-1), _blob_crown_margin_inf);

            cv::dilate(blob_dilated_inf, blob_crown, cross, cv::Point(-1,-1), _blob_crown_margin_sup - _blob_crown_margin_inf);
            // Subtract blob_crown from blob_crown
            cv::bitwise_not(blob_dilated_inf, blob_dilated_inf);
            cv::bitwise_and(blob_crown, blob_dilated_inf, blob_crown);
        }

        // Blob hue (color) subimage
        cv::Mat blob_hue = (*h)(blob_rect);
        // Binarize blob hue according to the valid laser hue range
        if(_hue_min <= _hue_max)
            blob_hue = blob_hue >= _hue_min & blob_hue <= _hue_max;
        else /*if(_hue_min > _hue_max)*/
            blob_hue = blob_hue >= _hue_min | blob_hue <= _hue_max;

        // Count crown pixels and crown pixels with valid colors
        int nb_crown_pixels = 0;
        int nb_valid_crown_pixels = 0;
        for(int i = 0, height = blob_rect.height; i < height; ++i)
            for(int j = 0, width = blob_rect.width; j < width; ++j) {
                if(blob_crown.at<uchar>(i, j) == 0)
                    // Not a crawn pixel
                    continue;
                ++nb_crown_pixels;

                if(blob_hue.at<uchar>(i, j) == 0)
                    // Color of the crown pixel isn't valid
                    continue;
                ++nb_valid_crown_pixels;
            }

        // Chech if threre's enough valid crawn pixels and compute the laser blob center, if any.
        cv::Moments moments = cv::moments(contour);
        if(moments.m00 > 0. && nb_crown_pixels > 0 && static_cast<double>(nb_valid_crown_pixels) / nb_crown_pixels >= _blob_crown_valid_pixels_part_min) {
            if(_emit_filtered_images) {
                // color output (BGR format)
                cv::Mat blob_with_crown(blob_rect.size(), CV_8UC3, cv::Scalar(0, 0, 0));
                for(int i = 0, height = blob_rect.height; i < height; ++i)
                    for(int j = 0, width = blob_rect.width; j < width; ++j) {
                        if(blob.at<uchar>(i, j) != 0) {
                            blob_with_crown.at<cv::Vec3b>(i, j)[0] = 255;
                            blob_with_crown.at<cv::Vec3b>(i, j)[1] = 255;
                            blob_with_crown.at<cv::Vec3b>(i, j)[2] = 255;
                        }
                        if(blob_crown.at<uchar>(i, j) != 0) {
                            if(blob_hue.at<uchar>(i, j) == 0)
                                blob_with_crown.at<cv::Vec3b>(i, j)[2] = 255;
                            else
                                blob_with_crown.at<cv::Vec3b>(i, j)[1] = 255;
                        }
                    }
                emit laserBlobAvailable(cvMat2QImage(blob_with_crown));
            }
            emit laserPosition(center(moments));
            return;
        }
    }

    emit laserPosition(QPointF(), false);
}

void LaserDetector::setHighestBrightnessMin(int min)
{
    Q_ASSERT(min >= 0 && min <= 255);

    _highest_brightness_min = min;
}

void LaserDetector::setRelativeBrightnessMin(double min)
{
    Q_ASSERT(min >= 0. && min <= 1.);

    _relative_brightness_min = min;
}

void LaserDetector::setBlobClosingSize(uint size)
{
    _blob_closing_size = size;
}

void LaserDetector::setNbBlobsMax(int max)
{
    Q_ASSERT(max > 0);

    _nb_blobs_max = max;
}

void LaserDetector::setBlobCrownMargins(int inf, int sup)
{
    Q_ASSERT(inf >= 0 && inf < sup);

    _blob_crown_margin_inf = inf;
    _blob_crown_margin_sup = sup;
}

void LaserDetector::setBlobPerimeterRange(uint min, uint max)
{
    Q_ASSERT(min > 0 && min < max);

    _blob_perimeter_min = min;
    _blob_perimeter_max = max;
}

void LaserDetector::setHueRange(uchar min, uchar max)
{
    Q_ASSERT(min < 180 && max < 180);

    _hue_min = min;
    _hue_max = max;
}
void LaserDetector::setBlobCrownValidPixelsPartMin(double min)
{
    Q_ASSERT(min >= 0. && min <= 1.);

    _blob_crown_valid_pixels_part_min = min;
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

QImage LaserDetector::cvMat2QImage(const cv::Mat& mat, bool binarize) const
{
    Q_ASSERT(mat.type() == CV_8UC1 || mat.type() == CV_8UC3);

    if(mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, CV_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step1(), QImage::Format_RGB888).copy();
    } else if(binarize) {
        cv::Mat normalized = mat > 0;
        return QImage(normalized.data, normalized.cols, normalized.rows, normalized.step, QImage::Format_Grayscale8).copy();
    }
    /*else*/ return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8).copy();
}

} // namespace laser_painter
