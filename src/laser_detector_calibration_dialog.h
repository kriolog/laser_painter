#ifndef LASER_DETECTOR_CALIBRATION_DIALOG
#define LASER_DETECTOR_CALIBRATION_DIALOG

#include <QDialog>

class QSpinBox;
class QDoubleSpinBox;
class QLabel;

namespace laser_painter {
    class LaserDetector;
}

namespace laser_painter {

class LaserDetectorCalibrationDialog: public QDialog
{
    Q_OBJECT

public:
    explicit LaserDetectorCalibrationDialog(
        LaserDetector* laser_detector,
        QWidget* parent = 0, Qt::WindowFlags f = 0
    );

    void writeSettings() const;

signals:
    void hueRangeChanged(uchar min, uchar max) const;
    void blobClosingSizeChanged(uint min) const;
    void blobPerimeterRangeChanged(uint min, uint max) const;
    void blobCrownMarginsChanged(uint min, uint max) const;
    void visible(bool visible) const;

public slots:
    void setVisible(bool visible);

private slots:
    void computeHueRange() const;
    void emitBlobClosingSizeChanged() const;
    void emitBlobPerimeterRangeChanged() const;
    void emitBlobCrownMarginsChanged() const;

private:
    inline void computeRange(int mean, int span, int& min, int& max) const;

private:
    QSpinBox* _highest_brightness_min_sb;
    QDoubleSpinBox* _relative_brightness_min_sb;
    QSpinBox* _blob_closing_size_sb;
    QSpinBox* _nb_blobs_max_sb;
    QSpinBox* _blob_perimeter_min_sb;
    QSpinBox* _blob_perimeter_max_sb;
    QSpinBox* _blob_crown_margin_inf_sb;
    QSpinBox* _blob_crown_margin_sup_sb;
    QSpinBox* _hue_mean_sb;
    QSpinBox* _hue_span_sb;
    QDoubleSpinBox* _blob_crown_valid_pixels_part_min_sb;
};

} // namespace laser_painter

#endif // LASER_DETECTOR_CALIBRATION_DIALOG