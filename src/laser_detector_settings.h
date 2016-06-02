#ifndef LASER_DETECTOR_SETTINGS
#define LASER_DETECTOR_SETTINGS

#include <QWidget>

class QSpinBox;
class QLabel;

namespace laser_painter {
    class LaserDetector;
}

namespace laser_painter {

class LaserDetectorSettings: public QWidget
{
    Q_OBJECT

public:
    explicit LaserDetectorSettings(
        const LaserDetector& laser_detector,
        QWidget* parent = 0, Qt::WindowFlags f = 0
    );

signals:
    void hueRangeChanged(uchar min, uchar max) const;
    void saturationRangeChanged(uchar min, uchar max) const;
    void valueRangeChanged(uchar min, uchar max) const;
    void blobClosingSizeChanged(uint min) const;
    void blobAreaParamsChanged(bool enable, uint min, uint max) const;
    void blobCircularityParamsChanged(bool enable, double min, double max) const;
    void visible(bool visible) const;

public slots:
    void setVisible(bool visible);

private slots:
    void computeHueRange() const;
    void computeSaturationRange() const;
    void computeValueRange() const;
    void emitBlobClosingSizeChanged() const;
    void emitBlobAreaParamsChanged() const;
    void emitBlobCircularityParamsChanged() const;

private:
    inline void computeRange(int mean, int span, int& min, int& max) const;

private:
    QSpinBox* _hue_mean_sb;
    QSpinBox* _hue_span_sb;
    QSpinBox* _saturation_mean_sb;
    QSpinBox* _saturation_span_sb;
    QSpinBox* _value_mean_sb;
    QSpinBox* _value_span_sb;
    QSpinBox* _blob_closing_size_sb;
    QSpinBox* _blob_area_min_sb;
    QSpinBox* _blob_area_max_sb;
    QSpinBox* _blob_circularity_min_sb;
    QSpinBox* _blob_circularity_max_sb;

    // TODO: use lambdas instead
    QLabel* _blob_area_lb;
    QLabel* _blob_circularity_lb;
    QLabel* _blob_closing_lb;

private slots:
    // TODO: use lambdas instead
    void blobClosingSetEnabled(int enabled);
    void blobAreaLbSetEnabled(int enabled);
    void blobCircularitySetEnabled(int enabled);
};

} // namespace laser_painter

#endif // LASER_DETECTOR_SETTINGS