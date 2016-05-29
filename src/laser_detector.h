#ifndef LASER_DETECTOR_H
#define LASER_DETECTOR_H

#include <QObject>

class QImage;
class QPointF;
class QSize;

namespace cv {
    class Mat;
}

namespace laser_painter {

/// Detect a laser dot position (or its absence) in the input image.
class LaserDetector : public QObject
{
    Q_OBJECT

public:
    /// @param {hue,saturation,value}_{min,max} are parameters of threshold
    /// filters for HSV channels.
    /// Hue range is [0, 360/2) = [0, 180) to fit the uchar limits [0, 255].
    /// For saturation and value, min <= max. Hue is circular: for instance,
    /// range min = 179 and max = 1 means the red color with hue = {179, 0, 1}.
    ///
    /// @param blob_closing_size is a kernel size for morphological closing for
    /// the detected laser blob(s) candidates before filtering. If zero,
    /// no closing will be performed.
    ///
    /// @param with_area_filter filter by area range of the detected blob.
    /// min <= max.
    /// @param with_circularity_filter filter by circularity range of the
    /// detected blob. Circularity is 4*Pi*area/perimeter^2 and runs from
    /// zero (bar) to one (circle). min <= max.

    explicit LaserDetector
    (
        uchar hue_min = 0,
        uchar hue_max = 179,
        uchar saturation_min = 0,
        uchar saturation_max = 255,
        uchar value_min = 0,
        uchar value_max = 255,

        uint blob_closing_size = 0,

        bool with_area_filter = false,
        uint blob_area_min = 0,
        uint blob_area_max = 0,

        bool with_circularity_filter = false,
        uint blob_circularity_min = 0,
        uint blob_circularity_max = 0,

        QObject* parent = 0
    );

public slots:
    /// Run the detection for the input image @param image.
    /// @retval laserPosition signal
    void run(const QImage& image) const;

    /// @see LaserDetector()
    void setHueRange(uchar min, uchar max);
    /// @see LaserDetector()
    void setSaturationParams(uchar min, uchar max);
    /// @see LaserDetector()
    void setValueRange(uchar min, uchar max);
    /// @see LaserDetector()
    void setAreaFilterParams(bool enable, uint min = 0, uint max = 0);
    /// @see LaserDetector()
    void setCircularityFilterParams(bool enable, uint min = 0, uint max = 0);
    /// @see LaserDetector()
    void setBlobClosingSize(uint size);

signals:
    /// Emit a laser dot position @param pos in the input image of
    /// size @param canvas_size and @param found = true if one and only one
    /// laser dot candidate has been found, otherwise return @param found = false.
    void laserPosition(const QPointF& pos, bool found = true) const;

private:
    // Convert QImage @param image to a HSV cv::Mat
    cv::Mat QImage2cvMat(const QImage& image) const;

    // Binary threshold {0, 1} of the @param src in the range [min, max].
    // Result is saved to the @param dst.
    // @param src and @param dst should have CV_8U type.
    inline void thresholdInRnage(const cv::Mat& src, cv::Mat& dst, uchar min, uchar max) const;

private:
    // HSV ranges for thresholding and other parameters.
    uchar _hue_min;
    uchar _hue_max;
    uchar _saturation_min;
    uchar _saturation_max;
    uchar _value_min;
    uchar _value_max;

    uint _blob_closing_size;

    bool _with_area_filter;
    uint _blob_area_min;
    uint _blob_area_max;

    bool _with_circularity_filter;
    uint _blob_circularity_min;
    uint _blob_circularity_max;
};

} // namespace laser_painter

#endif // LASER_DETECTOR_H