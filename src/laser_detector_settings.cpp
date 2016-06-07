#include "laser_detector_settings.h"

#include <QSettings>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>

#include "laser_detector_calibration_dialog.h"

namespace laser_painter {

LaserDetectorSettings::LaserDetectorSettings(
    LaserDetectorCalibrationDialog* calibration_dialog,
    QWidget* parent
)
    : QGroupBox(parent),
    _calibration_dg(calibration_dialog)
{
    Q_ASSERT(_calibration_dg);

    setTitle(tr("Detector Settings"));

    QSettings settings;

    QPushButton* calibration_bn = new QPushButton(tr("Calibration"));
    connect(calibration_bn, &QPushButton::clicked, this, &LaserDetectorSettings::showFocusCalibraitionDialog);

    _downscale_sb = new QDoubleSpinBox();
    _downscale_sb->setRange(1., 20.);
    _downscale_sb->setSingleStep(0.1);
    connect(_downscale_sb, SIGNAL(valueChanged(double)), this, SLOT(emitScaleChanged()));
    _downscale_sb->setValue(settings.value("LaserDetectorSettings/downscale", 1.).toDouble());
    QLabel* downscale_lb = new QLabel(tr("Downscale"));
    downscale_lb->setToolTip(tr("Downscale of the camera image for the laser dot\ndetector (ratio original image / scaled image).\nIncrease it if dot detection is slow."));
    downscale_lb->setBuddy(_downscale_sb);

    QHBoxLayout* downscale_lo = new QHBoxLayout();
    downscale_lo->addStretch();
    downscale_lo->addWidget(downscale_lb);
    downscale_lo->addWidget(_downscale_sb);

    QHBoxLayout* calibration_lo = new QHBoxLayout();
    calibration_lo->addStretch();
    calibration_lo->addWidget(calibration_bn);

    QVBoxLayout* main_lo = new QVBoxLayout();
    main_lo->addLayout(downscale_lo);
    main_lo->addLayout(calibration_lo);
    setLayout(main_lo);
}

void LaserDetectorSettings::writeSettings() const
{
    QSettings settings;

    settings.beginGroup("LaserDetectorSettings");

    settings.setValue("downscale", _downscale_sb->value());

    settings.endGroup();
}

void LaserDetectorSettings::showFocusCalibraitionDialog() const
{
    if(_calibration_dg->isVisible())
        // Set top level if already visible.
        // HACK: not an elegant solution, but standard ones rise()
        // and activateWindow() don't work.
        _calibration_dg->hide();
    _calibration_dg->show();
}

void LaserDetectorSettings::emitScaleChanged() const
{
    double scale = _downscale_sb->value();
    Q_ASSERT(scale >= 1.);
    emit scaleChanged(1. / scale);
}

} // namespace laser_painter