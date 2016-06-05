#include "laser_detector_settings.h"

#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

#include "laser_detector.h"
#include "image_widget.h"

namespace laser_painter {

LaserDetectorSettings::LaserDetectorSettings(
    LaserDetector* laser_detector,
    QWidget* parent, Qt::WindowFlags f
)
    : QWidget(parent, f)
{
    Q_ASSERT(laser_detector);

    QSettings settings;

    // Connections of settings with the laser detector
    connect(this, &LaserDetectorSettings::hueRangeChanged, laser_detector, &LaserDetector::setHueRange);
    connect(this, &LaserDetectorSettings::saturationRangeChanged, laser_detector, &LaserDetector::setSaturationRange);
    connect(this, &LaserDetectorSettings::valueRangeChanged, laser_detector, &LaserDetector::setValueRange);
    connect(this, &LaserDetectorSettings::blobClosingSizeChanged, laser_detector, &LaserDetector::setBlobClosingSize);
    connect(this, &LaserDetectorSettings::blobAreaParamsChanged, laser_detector, &LaserDetector::setBlobAreaParams);
    connect(this, &LaserDetectorSettings::blobCircularityParamsChanged, laser_detector, &LaserDetector::setBlobCircularityParams);
    // Laser detector emits binary images for settings calibration when the widget is visible.
    connect(this, &LaserDetectorSettings::visible, laser_detector, &LaserDetector::setEmitFilteredImages);


    // TODO : factorize hue, saturation, value settings creation.
    //////// HUE ////////
    // Hue mean value
    QSlider* hue_mean_sl = new QSlider(Qt::Horizontal);
    hue_mean_sl->setRange(0, 179);
    _hue_mean_sb = new QSpinBox();
    _hue_mean_sb->setRange(0, 179);
    connect(hue_mean_sl, SIGNAL(valueChanged(int)), _hue_mean_sb, SLOT(setValue(int)));
    connect(_hue_mean_sb, SIGNAL(valueChanged(int)), hue_mean_sl, SLOT(setValue(int)));
    QLabel* hue_mean_lb = new QLabel(tr("Mean"));
    hue_mean_lb->setBuddy(_hue_mean_sb);
    hue_mean_lb->setToolTip(tr("Hue corresponds to the laser wavelength.\nReference values:\n0 for a red laser\n60 for a green laser\n120 for a blue laser"));

    // Hue span
    QSlider* hue_span_sl = new QSlider(Qt::Horizontal);
    hue_span_sl->setRange(1, 180);
    _hue_span_sb = new QSpinBox();
    _hue_span_sb->setRange(1, 180);
    connect(hue_span_sl, SIGNAL(valueChanged(int)), _hue_span_sb, SLOT(setValue(int)));
    connect(_hue_span_sb, SIGNAL(valueChanged(int)), hue_span_sl, SLOT(setValue(int)));
    QLabel* hue_span_lb = new QLabel(tr("Span"));
    hue_span_lb->setBuddy(_hue_span_sb);
    hue_span_lb->setToolTip(tr("Hue span = maximum - minimum + 1."));

    connect(_hue_mean_sb, SIGNAL(valueChanged(int)), this, SLOT(computeHueRange()));
    connect(_hue_span_sb, SIGNAL(valueChanged(int)), this, SLOT(computeHueRange()));
    _hue_mean_sb->setValue(settings.value("LaserDetectorSettings/hue_mean", 0).toInt());
    _hue_span_sb->setValue(settings.value("LaserDetectorSettings/hue_span", 42).toInt());

    ImageWidget* hue_img_wgt = new ImageWidget();
    connect(laser_detector, &LaserDetector::hueFilteredAvailable, hue_img_wgt, &ImageWidget::setImage);

    QHBoxLayout* hue_mean_lo = new QHBoxLayout();
    hue_mean_lo->addWidget(hue_mean_lb);
    hue_mean_lo->addWidget(hue_mean_sl);
    hue_mean_lo->addWidget(_hue_mean_sb);

    QHBoxLayout* hue_span_lo = new QHBoxLayout();
    hue_span_lo->addWidget(hue_span_lb);
    hue_span_lo->addWidget(hue_span_sl);
    hue_span_lo->addWidget(_hue_span_sb);

    QVBoxLayout* hue_lo = new QVBoxLayout();
    hue_lo->addLayout(hue_mean_lo);
    hue_lo->addLayout(hue_span_lo);
    hue_lo->addWidget(hue_img_wgt);
    QGroupBox* hue_gb = new QGroupBox(tr("Hue"));
    hue_gb->setLayout(hue_lo);

    //////// SATURATION ////////
    // Saturation mean value
    QSlider* saturation_mean_sl = new QSlider(Qt::Horizontal);
    saturation_mean_sl->setRange(0, 255);
    _saturation_mean_sb = new QSpinBox();
    _saturation_mean_sb->setRange(0, 255);
    connect(saturation_mean_sl, SIGNAL(valueChanged(int)), _saturation_mean_sb, SLOT(setValue(int)));
    connect(_saturation_mean_sb, SIGNAL(valueChanged(int)), saturation_mean_sl, SLOT(setValue(int)));
    QLabel* saturation_mean_lb = new QLabel(tr("Mean"));
    saturation_mean_lb->setBuddy(_saturation_mean_sb);

    // Saturation span
    QSlider* saturation_span_sl = new QSlider(Qt::Horizontal);
    saturation_span_sl->setRange(1, 256);
    _saturation_span_sb = new QSpinBox();
    _saturation_span_sb->setRange(1, 256);
    connect(saturation_span_sl, SIGNAL(valueChanged(int)), _saturation_span_sb, SLOT(setValue(int)));
    connect(_saturation_span_sb, SIGNAL(valueChanged(int)), saturation_span_sl, SLOT(setValue(int)));
    QLabel* saturation_span_lb = new QLabel(tr("Span"));
    saturation_span_lb->setBuddy(_saturation_span_sb);
    saturation_span_lb->setToolTip(tr("Saturation span = maximum - minimum + 1."));

    connect(_saturation_mean_sb, SIGNAL(valueChanged(int)), this, SLOT(computeSaturationRange()));
    connect(_saturation_span_sb, SIGNAL(valueChanged(int)), this, SLOT(computeSaturationRange()));
    _saturation_mean_sb->setValue(settings.value("LaserDetectorSettings/saturation_mean", 0).toInt());
    _saturation_span_sb->setValue(settings.value("LaserDetectorSettings/saturation_span", 42).toInt());

    ImageWidget* saturation_img_wgt = new ImageWidget();
    connect(laser_detector, &LaserDetector::saturationFilteredAvailable, saturation_img_wgt, &ImageWidget::setImage);

    QHBoxLayout* saturation_mean_lo = new QHBoxLayout();
    saturation_mean_lo->addWidget(saturation_mean_lb);
    saturation_mean_lo->addWidget(saturation_mean_sl);
    saturation_mean_lo->addWidget(_saturation_mean_sb);

    QHBoxLayout* saturation_span_lo = new QHBoxLayout();
    saturation_span_lo->addWidget(saturation_span_lb);
    saturation_span_lo->addWidget(saturation_span_sl);
    saturation_span_lo->addWidget(_saturation_span_sb);

    QVBoxLayout* saturation_lo = new QVBoxLayout();
    saturation_lo->addLayout(saturation_mean_lo);
    saturation_lo->addLayout(saturation_span_lo);
    saturation_lo->addWidget(saturation_img_wgt);
    _saturation_gb = new QGroupBox(tr("Saturation"));
    _saturation_gb->setCheckable(true);
    connect(_saturation_gb, &QGroupBox::toggled, laser_detector, &LaserDetector::setWithSaturation);
    _saturation_gb->setChecked(settings.value("LaserDetectorSettings/with_saturation", true).toBool());
    _saturation_gb->setLayout(saturation_lo);

    //////// VALUE ////////
    // Value mean value
    QSlider* value_mean_sl = new QSlider(Qt::Horizontal);
    value_mean_sl->setRange(0, 255);
    _value_mean_sb = new QSpinBox();
    _value_mean_sb->setRange(0, 255);
    connect(value_mean_sl, SIGNAL(valueChanged(int)), _value_mean_sb, SLOT(setValue(int)));
    connect(_value_mean_sb, SIGNAL(valueChanged(int)), value_mean_sl, SLOT(setValue(int)));
    QLabel* value_mean_lb = new QLabel(tr("Mean"));
    value_mean_lb->setBuddy(_value_mean_sb);

    // Value span
    QSlider* value_span_sl = new QSlider(Qt::Horizontal);
    value_span_sl->setRange(1, 256);
    _value_span_sb = new QSpinBox();
    _value_span_sb->setRange(1, 256);
    connect(value_span_sl, SIGNAL(valueChanged(int)), _value_span_sb, SLOT(setValue(int)));
    connect(_value_span_sb, SIGNAL(valueChanged(int)), value_span_sl, SLOT(setValue(int)));
    QLabel* value_span_lb = new QLabel(tr("Span"));
    value_span_lb->setBuddy(_value_span_sb);
    value_span_lb->setToolTip(tr("Value span = maximum - minimum + 1."));

    connect(_value_mean_sb, SIGNAL(valueChanged(int)), this, SLOT(computeValueRange()));
    connect(_value_span_sb, SIGNAL(valueChanged(int)), this, SLOT(computeValueRange()));
    _value_mean_sb->setValue(settings.value("LaserDetectorSettings/value_mean", 0).toInt());
    _value_span_sb->setValue(settings.value("LaserDetectorSettings/value_span", 42).toInt());

    ImageWidget* value_img_wgt = new ImageWidget();
    connect(laser_detector, &LaserDetector::valueFilteredAvailable, value_img_wgt, &ImageWidget::setImage);

    QHBoxLayout* value_mean_lo = new QHBoxLayout();
    value_mean_lo->addWidget(value_mean_lb);
    value_mean_lo->addWidget(value_mean_sl);
    value_mean_lo->addWidget(_value_mean_sb);

    QHBoxLayout* value_span_lo = new QHBoxLayout();
    value_span_lo->addWidget(value_span_lb);
    value_span_lo->addWidget(value_span_sl);
    value_span_lo->addWidget(_value_span_sb);

    QVBoxLayout* value_lo = new QVBoxLayout();
    value_lo->addLayout(value_mean_lo);
    value_lo->addLayout(value_span_lo);
    value_lo->addWidget(value_img_wgt);
    QGroupBox* value_gb = new QGroupBox(tr("Value"));
    value_gb->setLayout(value_lo);


    //////// BLOB FILTERS ////////
    //// BLOB CLOSING SIZE ////
    _blob_closing_lb = new QLabel(tr("Fill holes:"));
    _blob_closing_lb->setToolTip(tr("Fill small holes and non-convex parts\n of the detected blobs (morphological closing)."));
    _blob_closing_size_sb = new QSpinBox();
    _blob_closing_size_sb->setRange(0, 25);
    QLabel* blob_closing_size_lb = new QLabel(tr("Size"));
    blob_closing_size_lb->setBuddy(_blob_closing_size_sb);
    blob_closing_size_lb->setToolTip(tr("Radius of holes. If zero, the filter won't be applied."));

    connect(_blob_closing_size_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobClosingSizeChanged()));
    connect(_blob_closing_size_sb, SIGNAL(valueChanged(int)), this, SLOT(blobClosingSetEnabled(int)));
    _blob_closing_size_sb->setValue(settings.value("LaserDetectorSettings/blob_closing_size", 0).toInt());
    blobClosingSetEnabled(_blob_closing_size_sb->value());

    //// BLOB AREA ////
    _blob_area_lb = new QLabel(tr("Blob size:"));
    _blob_area_lb->setToolTip(tr("Bandpass filter for the size (area in pixels) of blobs."));

    _blob_area_min_sb = new QSpinBox();
    _blob_area_min_sb->setRange(0, 9999);
    QLabel* blob_area_min_lb = new QLabel(tr("Min"));
    blob_area_min_lb->setBuddy(_blob_area_min_sb);
    _blob_area_max_sb = new QSpinBox();
    _blob_area_max_sb->setMaximum(9999);
    QLabel* blob_area_max_lb = new QLabel(tr("Max"));
    blob_area_max_lb->setBuddy(_blob_area_max_sb);
    blob_area_max_lb->setToolTip(tr("Maximum size. If zero, the filter won't be applied."));
    connect(_blob_area_min_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobAreaParamsChanged()));
    connect(_blob_area_max_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobAreaParamsChanged()));
    connect(_blob_area_max_sb, SIGNAL(valueChanged(int)), this, SLOT(blobAreaLbSetEnabled(int)));
    _blob_area_min_sb->setValue(settings.value("LaserDetectorSettings/blob_area_min", 0).toInt());
    _blob_area_max_sb->setValue(settings.value("LaserDetectorSettings/blob_area_max", 0).toInt());
    blobAreaLbSetEnabled(_blob_area_max_sb->value());

    // Blob closing and blob area are in the same horisontal layout.
    QHBoxLayout* blob_closing_area_lo = new QHBoxLayout();
    QHBoxLayout* blob_area_lo = new QHBoxLayout();
    blob_closing_area_lo->addWidget(_blob_closing_lb);
    blob_closing_area_lo->addWidget(blob_closing_size_lb);
    blob_closing_area_lo->addWidget(_blob_closing_size_sb);
    blob_closing_area_lo->addSpacing(32);
    blob_closing_area_lo->addWidget(_blob_area_lb);
    blob_closing_area_lo->addWidget(blob_area_min_lb);
    blob_closing_area_lo->addWidget(_blob_area_min_sb);
    blob_closing_area_lo->addWidget(blob_area_max_lb);
    blob_closing_area_lo->addWidget(_blob_area_max_sb);
    blob_closing_area_lo->addStretch();

    //// BLOB CIRCULARITY ////
    _blob_circularity_lb = new QLabel(tr("Blob circularity:"));
    _blob_circularity_lb->setToolTip(tr("Bandpass filter for the circularity (similarity\nto circles) of blobs. 0% is a bar case,\n100% is a circle case."));

    _blob_circularity_min_sb = new QSpinBox();
    _blob_circularity_min_sb->setSuffix("%");
    _blob_circularity_min_sb->setRange(0, 100);
    QLabel* blob_circularity_min_lb = new QLabel(tr("Min"));
    blob_circularity_min_lb->setBuddy(_blob_circularity_min_sb);
    _blob_circularity_max_sb = new QSpinBox();
    _blob_circularity_max_sb->setSuffix("%");
    _blob_circularity_max_sb->setMaximum(100);
    QLabel* blob_circularity_max_lb = new QLabel(tr("Max"));
    blob_circularity_max_lb->setBuddy(_blob_circularity_max_sb);
    blob_circularity_max_lb->setToolTip(tr("Maximum circularity. If zero, the filter won't be applied."));
    connect(_blob_circularity_min_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobCircularityParamsChanged()));
    connect(_blob_circularity_max_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobCircularityParamsChanged()));
    connect(_blob_circularity_max_sb, SIGNAL(valueChanged(int)), this, SLOT(blobCircularitySetEnabled(int)));
    _blob_circularity_min_sb->setValue(settings.value("LaserDetectorSettings/blob_circularity_min", 0).toInt());
    _blob_circularity_max_sb->setValue(settings.value("LaserDetectorSettings/blob_circularity_max", 0).toInt());
    blobCircularitySetEnabled(_blob_circularity_max_sb->value());

    QHBoxLayout* blob_circularity_lo = new QHBoxLayout();
    blob_circularity_lo->addWidget(_blob_circularity_lb);
    blob_circularity_lo->addWidget(blob_circularity_min_lb);
    blob_circularity_lo->addWidget(_blob_circularity_min_sb);
    blob_circularity_lo->addWidget(blob_circularity_max_lb);
    blob_circularity_lo->addWidget(_blob_circularity_max_sb);
    blob_circularity_lo->addStretch();

    ImageWidget* blobs_img_wgt = new ImageWidget();
    connect(laser_detector, &LaserDetector::blobsFilteredAvailable, blobs_img_wgt, &ImageWidget::setImage);

    QVBoxLayout* blob_filters_lo = new QVBoxLayout();
    blob_filters_lo->addLayout(blob_closing_area_lo);
    blob_filters_lo->addLayout(blob_circularity_lo);
    blob_filters_lo->addWidget(blobs_img_wgt);
    QGroupBox* blob_filters_gb = new QGroupBox(tr("Blob Filters"));
    blob_filters_gb->setLayout(blob_filters_lo);


    /// Advice
    QLabel* advice_lb = new QLabel(tr("General advice: try to keep the laser dot as white as possible for each HSV channel and make the background of channels as dissimilar as possible."));
    advice_lb->setWordWrap(true);


    QGridLayout* main_lo = new QGridLayout();
    main_lo->addWidget(advice_lb, 0, 0, 1, 2);
    main_lo->addWidget(hue_gb, 1, 0);
    main_lo->addWidget(_saturation_gb, 1, 1);
    main_lo->addWidget(value_gb, 2, 0);
    main_lo->addWidget(blob_filters_gb, 2, 1);
    main_lo->setRowStretch(0, 1);
    main_lo->setRowStretch(1, 1024);
    main_lo->setRowStretch(2, 1024);
    main_lo->setColumnStretch(0, 1);
    main_lo->setColumnStretch(1, 1);
    setLayout(main_lo);
}

void LaserDetectorSettings::setVisible(bool visible)
{
    emit this->visible(visible);
    QWidget::setVisible(visible);
}

void LaserDetectorSettings::computeHueRange() const
{
    int hue_min, hue_max;
    computeRange(_hue_mean_sb->value(), _hue_span_sb->value(), hue_min, hue_max);
    if(hue_min < 0)
        hue_min = hue_min + 180;
    if(hue_max >= 180)
        hue_max = hue_max - 180;

    Q_ASSERT(hue_min >= 0 && hue_min < 180 && hue_max >= 0 && hue_max < 180);
    emit hueRangeChanged(hue_min, hue_max);
}

void LaserDetectorSettings::computeSaturationRange() const
{
    int saturation_min, saturation_max;
    computeRange(_saturation_mean_sb->value(), _saturation_span_sb->value(), saturation_min, saturation_max);

    Q_ASSERT(saturation_min <= saturation_max);
    emit saturationRangeChanged(qMax(0, saturation_min), qMin(255, saturation_max));
}

void LaserDetectorSettings::computeValueRange() const
{
    int value_min, value_max;
    computeRange(_value_mean_sb->value(), _value_span_sb->value(), value_min, value_max);

    Q_ASSERT(value_min <= value_max);
    emit valueRangeChanged(qMax(0, value_min), qMin(255, value_max));
}

void LaserDetectorSettings::computeRange(int mean, int span, int& min, int& max ) const
{
    min = mean - (span - 1) / 2;
    max = mean + span / 2;
}

void LaserDetectorSettings::emitBlobClosingSizeChanged() const
{
    int size = _blob_closing_size_sb->value();
    Q_ASSERT(size >=0);
    if(size > 0)
        size = 2 * size + 1;

    emit blobClosingSizeChanged(size);
}

void LaserDetectorSettings::emitBlobAreaParamsChanged() const
{
    int min = _blob_area_min_sb->value();
    _blob_area_max_sb->setMinimum(min);
    int max = _blob_area_max_sb->value();

    Q_ASSERT(min >= 0 && min <= max);
    emit blobAreaParamsChanged(max == 0 ? false : true, min, max);
}

void LaserDetectorSettings::emitBlobCircularityParamsChanged() const
{
    int min = _blob_circularity_min_sb->value();
    _blob_circularity_max_sb->setMinimum(min);
    int max = _blob_circularity_max_sb->value();

    Q_ASSERT(min >=0. && min <= max);
    emit blobCircularityParamsChanged(max == 0 ? false : true, min / 100., max / 100.);
}

void LaserDetectorSettings::blobClosingSetEnabled(int enabled)
{
    _blob_closing_lb->setEnabled(enabled);
}

void LaserDetectorSettings::blobAreaLbSetEnabled(int enabled)
{
    _blob_area_lb->setEnabled(enabled);
}

void LaserDetectorSettings::blobCircularitySetEnabled(int enabled)
{
    _blob_circularity_lb->setEnabled(enabled);
}


void LaserDetectorSettings::writeSettings() const
{
    QSettings settings;
    settings.beginGroup("LaserDetectorSettings");

    settings.setValue("hue_mean", _hue_mean_sb->value());
    settings.setValue("hue_span", _hue_span_sb->value());
    settings.setValue("with_saturation", _saturation_gb->isChecked());
    settings.setValue("saturation_mean", _saturation_mean_sb->value());
    settings.setValue("saturation_span", _saturation_span_sb->value());
    settings.setValue("value_mean", _value_mean_sb->value());
    settings.setValue("value_span", _value_span_sb->value());

    settings.setValue("blob_closing_size", _blob_closing_size_sb->value());
    settings.setValue("blob_area_min", _blob_area_min_sb->value());
    settings.setValue("blob_area_max", _blob_area_max_sb->value());
    settings.setValue("blob_circularity_min", _blob_circularity_min_sb->value());
    settings.setValue("blob_circularity_max", _blob_circularity_max_sb->value());

    settings.endGroup();
}

} // namespace laser_painter