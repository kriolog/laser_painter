#ifndef LASER_DETECTOR_SETTINGS
#define LASER_DETECTOR_SETTINGS

#include <QGroupBox>

class QDoubleSpinBox;

namespace laser_painter {
    class LaserDetectorCalibrationDialog;
}

namespace laser_painter {

class LaserDetectorSettings: public QGroupBox
{
    Q_OBJECT

public:
    explicit LaserDetectorSettings(
        LaserDetectorCalibrationDialog* calibration_dialog,
        QWidget* parent = 0
    );

    void writeSettings() const;

signals:
    // scale <= 1.
    void scaleChanged(qreal scale) const;

public slots:
    // Emits a scaleChanged() signal with the current scale
    // HACK: make it public to resolve a connection after construcion problem
    void emitScaleChanged() const;

private slots:
    void showFocusCalibraitionDialog() const;

private:
    LaserDetectorCalibrationDialog* _calibration_dg;
    QDoubleSpinBox* _downscale_sb;
};

} // namespace laser_painter

#endif // LASER_DETECTOR_SETTINGS