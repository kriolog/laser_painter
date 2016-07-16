#ifndef LASER_DETECTOR_H
#define LASER_DETECTOR_H

#include <QObject>

class QImage;
class QPointF;
class QSize;

namespace cv {
    class Mat;
    class Moments;
}

namespace laser_painter {

/// Detect a laser dot position (or its absence) in the input image.
class LaserDetector : public QObject
{
    Q_OBJECT

public:
    /// @param hue_{min,max} are parameters for a threshold filter of the hue
    /// channel (HSV color scheme).
    /// Hue range is [0, 360/2) = [0, 180) to fit the uchar limits [0, 255].
    /// Hue is circular: for instance, range min = 179 and max = 1 corresponds
    /// to the red color with hue = {179, 0, 1}.
    ///
    /// @param blob_closing_size is a kernel size for morphological closing for
    /// the detected laser blob(s) candidates before filtering. If zero,
    /// no closing will be performed.
    ///
    /// @param emit_filtered_images emit thresholded blobs and a detected blob,
    /// if any.
    explicit LaserDetector
    (
        QObject* parent = 0,

        uchar highest_brightness_min = 200,
        double relative_brightness_min = 0.9,
        uint _blob_closing_size = 0,
        uint nb_blobs_max = 16,
        uint blob_perimeter_min = 1,
        uint blob_perimeter_max = 512,
        int blob_crown_margin_inf = 1,
        int blob_crown_margin_sup = 3,
        uchar hue_min = 0,
        uchar hue_max = 179,
        double blob_crown_valid_pixels_part_min = 0.5,

        bool emit_filtered_images = false
    );

public slots:
    /// Run the detection for the input image @param image.
    /// @retval laserPosition signal
    void run(const QImage& image) const;

    void setHighestBrightness(uchar max);
    void setRelativeBrightnessMin(double min);
    void setBlobClosingSize(uint size);
    void setNbBlobsMax(double max);
    void setBlobCrownMargins(int inf, int sup);
    void setBlobPerimeterRange(uint min, uint max);
    void setHueRange(uchar min, uchar max);
    void setBlobCrownValidPixelsPartMin(double min);

    void setEmitFilteredImages(bool do_emit);

signals:
    /// Emit a laser dot position @param pos in the input image coordinates,
    /// @param found = true if laser dot position is found, false otherwise.
    void laserPosition(const QPointF& pos, bool found = true) const;
    /// Binary image of the filtered hue component.
    void blobsAvailable(const QImage& hue) const;
    void laserBlobAvailable(const QImage& blobs) const;

    void warning(const QString& text) const;

private:
    // Compute center by moments. Area (m00) should be positive.
    inline QPointF center(const cv::Moments& moments) const;
    // Convert a QImage @param image to a RGB cv::Mat
    cv::Mat QImage2cvMat(const QImage& image) const;
    // Convert cv::Mat to a QImage (valid formats are CV_8U1 and CV_8U3 (BGR))
    // If @param binarize is true and format is CV_8U1, nowmalize @param mat
    // to obtain a black/white (0/255) image.
    QImage cvMat2QImage(const cv::Mat& mat, bool binarize = false) const;

private:
    uchar _highest_brightness_min;
    double _relative_brightness_min;
    uint _blob_closing_size;
    uint _nb_blobs_max;
    uint _blob_perimeter_min;
    uint _blob_perimeter_max;
    uint _blob_crown_margin_inf;
    uint _blob_crown_margin_sup;
    uchar _hue_min;
    uchar _hue_max;
    // Crown pixels with valid colors (defined by range of hue_{min,max}).
    double _blob_crown_valid_pixels_part_min;

    bool _emit_filtered_images;
};

} // namespace laser_painter

#endif // LASER_DETECTOR_H