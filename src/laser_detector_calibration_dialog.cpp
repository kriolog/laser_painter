#include "laser_detector_calibration_dialog.h"

#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

#include "laser_detector.h"
#include "image_widget.h"

namespace laser_painter {

LaserDetectorCalibrationDialog::LaserDetectorCalibrationDialog(
    LaserDetector* laser_detector,
    QWidget* parent, Qt::WindowFlags f
)
    : QDialog(parent, f)
{
    Q_ASSERT(laser_detector);

    QSettings settings;

    setWindowTitle(tr("Laser Detector Calibration"));
    setMinimumSize(1024, 768);
    setFixedSize(settings.value("LaserDetectorCalibrationDialog/dialog_size", minimumSize()).toSize());

    // Connections of settings with the laser detector
    connect(this, &LaserDetectorCalibrationDialog::hueRangeChanged, laser_detector, &LaserDetector::setHueRange);
    connect(this, &LaserDetectorCalibrationDialog::blobClosingSizeChanged, laser_detector, &LaserDetector::setBlobClosingSize);
    connect(this, &LaserDetectorCalibrationDialog::blobPerimeterRangeChanged, laser_detector, &LaserDetector::setBlobPerimeterRange);
    connect(this, &LaserDetectorCalibrationDialog::blobCrownMarginsChanged, laser_detector, &LaserDetector::setBlobCrownMargins);
    // Laser detector emits binary images for settings calibration when the widget is visible.
    connect(this, &LaserDetectorCalibrationDialog::visible, laser_detector, &LaserDetector::setEmitFilteredImages);


    //////// Highest brightness threshold ////////
    _highest_brightness_min_sb = new QSpinBox();
    _highest_brightness_min_sb->setRange(0, 255);
    QLabel* highest_brightness_min_lb = new QLabel(tr("Minimum highest brightness:"));
    highest_brightness_min_lb->setBuddy(_highest_brightness_min_sb);
    highest_brightness_min_lb->setToolTip(tr("When brightness of the brightest\npixel is less than this threshold the image\nwon't be processed."));
    connect(_highest_brightness_min_sb, SIGNAL(valueChanged(int)), laser_detector, SLOT(setHighestBrightnessMin(int)));
    _highest_brightness_min_sb->setValue(settings.value("LaserDetectorCalibrationDialog/highest_brightness_min", 200).toInt());
    QHBoxLayout* highest_brightness_min_lo = new QHBoxLayout();
    highest_brightness_min_lo->addStretch();
    highest_brightness_min_lo->addWidget(highest_brightness_min_lb);
    highest_brightness_min_lo->addWidget(_highest_brightness_min_sb);

    //////// Relative brightness threshold ////////
    _relative_brightness_min_sb = new QDoubleSpinBox();
    _relative_brightness_min_sb->setRange(0.5, 1.0);
    _relative_brightness_min_sb->setDecimals(2);
    _relative_brightness_min_sb->setSingleStep(0.01);
    QLabel* relative_brightness_min_lb = new QLabel(tr("Minimum relative brightness:"));
    relative_brightness_min_lb->setBuddy(_relative_brightness_min_sb);
    relative_brightness_min_lb->setToolTip(tr("Pixels with relative brightness >= this threshold\nwill be considered as blob pixels."));
    connect(_relative_brightness_min_sb, SIGNAL(valueChanged(double)), laser_detector, SLOT(setRelativeBrightnessMin(double)));
    _relative_brightness_min_sb->setValue(settings.value("LaserDetectorCalibrationDialog/relative_brightness_min", 0.9).toDouble());
    QHBoxLayout* relative_brightness_min_lo = new QHBoxLayout();
    relative_brightness_min_lo->addStretch();
    relative_brightness_min_lo->addWidget(relative_brightness_min_lb);
    relative_brightness_min_lo->addWidget(_relative_brightness_min_sb);

    //// BLOB CLOSING SIZE ////
    _blob_closing_size_sb = new QSpinBox();
    _blob_closing_size_sb->setRange(0, 9);
    QLabel* blob_closing_size_lb = new QLabel(tr("Fill holes with radius:"));
    blob_closing_size_lb->setToolTip(tr("Fill small holes and non-convex parts\n of the detected blobs (morphological closing).\nValue is the maximum radius of holes\n(if zero, the filter won't be applied)."));
    blob_closing_size_lb->setBuddy(_blob_closing_size_sb);
    connect(_blob_closing_size_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobClosingSizeChanged()));
    _blob_closing_size_sb->setValue(settings.value("LaserDetectorCalibrationDialog/blob_closing_size", 2).toInt());
    QHBoxLayout* blob_closing_size_lo = new QHBoxLayout();
    blob_closing_size_lo->addStretch();
    blob_closing_size_lo->addWidget(blob_closing_size_lb);
    blob_closing_size_lo->addWidget(_blob_closing_size_sb);

    //// Maximum number of blobs ////
    _nb_blobs_max_sb = new QSpinBox();
    _nb_blobs_max_sb->setRange(1, 9999);
    QLabel* nb_blobs_max_lb = new QLabel(tr("Maximum number of blobs:"));
    nb_blobs_max_lb->setToolTip(tr("When the number of blobs exceeds this threshold\nthe image won't be processed."));
    nb_blobs_max_lb->setBuddy(_nb_blobs_max_sb);
    connect(_nb_blobs_max_sb, SIGNAL(valueChanged(int)), laser_detector, SLOT(setNbBlobsMax(int)));
    _nb_blobs_max_sb->setValue(settings.value("LaserDetectorCalibrationDialog/nb_blobs_max", 99).toInt());
    QHBoxLayout* nb_blobs_max_lo = new QHBoxLayout();
    nb_blobs_max_lo->addStretch();
    nb_blobs_max_lo->addWidget(nb_blobs_max_lb);
    nb_blobs_max_lo->addWidget(_nb_blobs_max_sb);

    //// Blob perimeter range ////
    QLabel* blob_perimeter_lb = new QLabel(tr("Blob perimeter range"));
    blob_perimeter_lb->setToolTip(tr("Range of perimeters of valid blobs"));
    // Min
    _blob_perimeter_min_sb= new QSpinBox();
    _blob_perimeter_min_sb->setRange(1, 99998);
    QLabel* blob_perimeter_min_lb = new QLabel(tr("Min:"));
    blob_perimeter_min_lb->setToolTip(tr("Minimum perimeter of valid blobs."));
    blob_perimeter_min_lb->setBuddy(_blob_perimeter_min_sb);
    // Max
    _blob_perimeter_max_sb= new QSpinBox();
    _blob_perimeter_max_sb->setRange(2, 99999);
    QLabel* blob_perimeter_max_lb = new QLabel(tr("Max:"));
    blob_perimeter_max_lb->setToolTip(tr("Maximum perimeter of valid blobs."));
    blob_perimeter_max_lb->setBuddy(_blob_perimeter_max_sb);
    // Connections
    connect(_blob_perimeter_min_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobPerimeterRangeChanged()));
    connect(_blob_perimeter_max_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobPerimeterRangeChanged()));
    _blob_perimeter_min_sb->setValue(settings.value("LaserDetectorCalibrationDialog/blob_perimeter_min", 1).toInt());
    _blob_perimeter_max_sb->setValue(settings.value("LaserDetectorCalibrationDialog/blob_perimeter_max", 999).toInt());
    // Layout
    QVBoxLayout* blob_perimeter_lo = new QVBoxLayout();
    QHBoxLayout* blob_perimeter_header_lo = new QHBoxLayout();
    QHBoxLayout* blob_perimeter_handlers_lo = new QHBoxLayout();
    blob_perimeter_lo->addLayout(blob_perimeter_header_lo);
    blob_perimeter_lo->addLayout(blob_perimeter_handlers_lo);
    blob_perimeter_header_lo->addStretch();
    blob_perimeter_header_lo->addWidget(blob_perimeter_lb);
    blob_perimeter_header_lo->addStretch();
    blob_perimeter_handlers_lo->addStretch();
    blob_perimeter_handlers_lo->addWidget(blob_perimeter_min_lb);
    blob_perimeter_handlers_lo->addWidget(_blob_perimeter_min_sb);
    blob_perimeter_handlers_lo->addWidget(blob_perimeter_max_lb);
    blob_perimeter_handlers_lo->addWidget(_blob_perimeter_max_sb);

    //// Blob crown margins ////
    QLabel* blob_crown_margins_lb = new QLabel(tr("Blob crown margins"));
    blob_perimeter_lb->setToolTip(tr("Distance from a blob to the inner and outer\nboundsries of the blob crown."));
    // Inf
    _blob_crown_margin_inf_sb = new QSpinBox();
    _blob_crown_margin_inf_sb->setRange(0, 98);
    QLabel* blob_crown_margin_inf_lb = new QLabel(tr("Inf:"));
    blob_crown_margin_inf_lb->setToolTip(tr("Distance from a blob to the inner boundary of the blob crown."));
    blob_crown_margin_inf_lb->setBuddy(_blob_crown_margin_inf_sb);
    // Sup
    _blob_crown_margin_sup_sb = new QSpinBox();
    _blob_crown_margin_sup_sb->setRange(1, 99);
    QLabel* blob_crown_margin_sup_lb = new QLabel(tr("Sup:"));
    blob_crown_margin_sup_lb->setToolTip(tr("Distance from a blob to the outer boundary of the blob crown."));
    blob_crown_margin_sup_lb->setBuddy(_blob_crown_margin_sup_sb);
    // Connections
    connect(_blob_crown_margin_inf_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobCrownMarginsChanged()));
    connect(_blob_crown_margin_sup_sb, SIGNAL(valueChanged(int)), this, SLOT(emitBlobCrownMarginsChanged()));
    _blob_crown_margin_inf_sb->setValue(settings.value("LaserDetectorCalibrationDialog/blob_crown_margin_inf", 0).toInt());
    _blob_crown_margin_sup_sb->setValue(settings.value("LaserDetectorCalibrationDialog/blob_crown_margin_sup", 1).toInt());
    // Layout
    QVBoxLayout* blob_crown_margins_lo = new QVBoxLayout();
    QHBoxLayout* blob_crown_margins_header_lo = new QHBoxLayout();
    QHBoxLayout* blob_crown_margins_handlers_lo = new QHBoxLayout();
    blob_crown_margins_lo->addLayout(blob_crown_margins_header_lo);
    blob_crown_margins_lo->addLayout(blob_crown_margins_handlers_lo);
    blob_crown_margins_header_lo->addStretch();
    blob_crown_margins_header_lo->addWidget(blob_crown_margins_lb);
    blob_crown_margins_header_lo->addStretch();
    blob_crown_margins_handlers_lo->addStretch();
    blob_crown_margins_handlers_lo->addWidget(blob_crown_margin_inf_lb);
    blob_crown_margins_handlers_lo->addWidget(_blob_crown_margin_inf_sb);
    blob_crown_margins_handlers_lo->addWidget(blob_crown_margin_sup_lb);
    blob_crown_margins_handlers_lo->addWidget(_blob_crown_margin_sup_sb);

    //////// HUE ////////
    QLabel* hue_lb = new QLabel(tr("Crown valid pixels color"));
    hue_lb->setToolTip(tr("Crown pixel is valid when its color (hue in HSV color scheme) is in the following range."));
    // Hue mean value
    QSlider* hue_mean_sl = new QSlider(Qt::Horizontal);
    hue_mean_sl->setRange(0, 179);
    _hue_mean_sb = new QSpinBox();
    _hue_mean_sb->setRange(0, 179);
    connect(hue_mean_sl, SIGNAL(valueChanged(int)), _hue_mean_sb, SLOT(setValue(int)));
    connect(_hue_mean_sb, SIGNAL(valueChanged(int)), hue_mean_sl, SLOT(setValue(int)));
    QLabel* hue_mean_lb = new QLabel(tr("Mean"));
    hue_mean_lb->setBuddy(_hue_mean_sb);
    hue_mean_lb->setToolTip(tr("Hue corresponds to the laser wavelength.\nReference values:\n0 for a red laser\n60 for a green laser\n120 for a blue laser."));
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
    // Connections
    connect(_hue_mean_sb, SIGNAL(valueChanged(int)), this, SLOT(computeHueRange()));
    connect(_hue_span_sb, SIGNAL(valueChanged(int)), this, SLOT(computeHueRange()));
    _hue_mean_sb->setValue(settings.value("LaserDetectorCalibrationDialog/hue_mean", 0).toInt());
    _hue_span_sb->setValue(settings.value("LaserDetectorCalibrationDialog/hue_span", 42).toInt());
    // Layout
    QVBoxLayout* hue_lo = new QVBoxLayout();
    QHBoxLayout* hue_header_lo = new QHBoxLayout();
    QHBoxLayout* hue_mean_lo = new QHBoxLayout();
    QHBoxLayout* hue_span_lo = new QHBoxLayout();
    hue_lo->addLayout(hue_header_lo);
    hue_lo->addLayout(hue_mean_lo);
    hue_lo->addLayout(hue_span_lo);
    hue_header_lo->addStretch();
    hue_header_lo->addWidget(hue_lb);
    hue_header_lo->addStretch();
    hue_mean_lo->addWidget(hue_mean_lb);
    hue_mean_lo->addWidget(hue_mean_sl);
    hue_mean_lo->addWidget(_hue_mean_sb);
    hue_span_lo->addWidget(hue_span_lb);
    hue_span_lo->addWidget(hue_span_sl);
    hue_span_lo->addWidget(_hue_span_sb);

    //// Blob crown valid pixels part ////
    _blob_crown_valid_pixels_part_min_sb = new QDoubleSpinBox();
    _blob_crown_valid_pixels_part_min_sb->setRange(0., 1.);
    _blob_crown_valid_pixels_part_min_sb->setDecimals(2);
    _blob_crown_valid_pixels_part_min_sb->setSingleStep(0.01);
    QLabel* blob_crown_valid_pixels_part_min_lb = new QLabel(tr("Crown valid pixels part:"));
    blob_crown_valid_pixels_part_min_lb->setToolTip(tr("Minimum part of pixels in the crown with good colors (hues)."));
    blob_crown_valid_pixels_part_min_lb->setBuddy(_blob_crown_valid_pixels_part_min_sb);
    connect(_blob_crown_valid_pixels_part_min_sb, SIGNAL(valueChanged(double)), laser_detector, SLOT(setBlobCrownValidPixelsPartMin(double)));
    _blob_crown_valid_pixels_part_min_sb->setValue(settings.value("LaserDetectorCalibrationDialog/blob_crown_valid_pixels_part_min", 0.66).toDouble());
    QHBoxLayout* blob_crown_valid_pixels_part_min_lo = new QHBoxLayout();
    blob_crown_valid_pixels_part_min_lo->addStretch();
    blob_crown_valid_pixels_part_min_lo->addWidget(blob_crown_valid_pixels_part_min_lb);
    blob_crown_valid_pixels_part_min_lo->addWidget(_blob_crown_valid_pixels_part_min_sb);


    ImageWidget* detected_blobs_img_wgt = new ImageWidget();
    connect(laser_detector, &LaserDetector::blobsAvailable, detected_blobs_img_wgt, &ImageWidget::setImage);
    QGroupBox* detected_blobs_gb = new QGroupBox(tr("Detected blob candidates"));
    detected_blobs_gb->setAlignment (Qt::AlignHCenter);
    QVBoxLayout* detected_blobs_lo = new QVBoxLayout();
    detected_blobs_lo->addWidget(detected_blobs_img_wgt);
    detected_blobs_gb->setLayout(detected_blobs_lo);

    ImageWidget* detected_blob_img_wgt = new ImageWidget();
    connect(laser_detector, &LaserDetector::laserBlobAvailable, detected_blob_img_wgt, &ImageWidget::setImage);
    QGroupBox* detected_blob_gb = new QGroupBox(tr("Detected (laser) blob with crown"));
    detected_blob_gb->setAlignment (Qt::AlignHCenter);
    QVBoxLayout* detected_blob_lo = new QVBoxLayout();
    detected_blob_lo->addWidget(detected_blob_img_wgt);
    detected_blob_gb->setLayout(detected_blob_lo);

    // Main layout
    QVBoxLayout* settings_lo = new QVBoxLayout();
    settings_lo->addLayout(highest_brightness_min_lo);
    settings_lo->addLayout(relative_brightness_min_lo);
    settings_lo->addLayout(blob_closing_size_lo);
    settings_lo->addLayout(nb_blobs_max_lo);
    settings_lo->addLayout(blob_perimeter_lo);
    settings_lo->addLayout(blob_crown_margins_lo);
    settings_lo->addLayout(hue_lo);
    settings_lo->addLayout(blob_crown_valid_pixels_part_min_lo);
    settings_lo->addStretch();

    QVBoxLayout* images_lo = new QVBoxLayout();
    images_lo->addWidget(detected_blobs_gb, 2);
    images_lo->addWidget(detected_blob_gb, 1);

    QHBoxLayout* main_lo = new QHBoxLayout();
    setLayout(main_lo);
    main_lo->addLayout(settings_lo, 1);
    main_lo->addLayout(images_lo, 2);
}

void LaserDetectorCalibrationDialog::setVisible(bool visible)
{
    emit this->visible(visible);
    QWidget::setVisible(visible);
}

void LaserDetectorCalibrationDialog::computeHueRange() const
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

void LaserDetectorCalibrationDialog::computeRange(int mean, int span, int& min, int& max ) const
{
    min = mean - (span - 1) / 2;
    max = mean + span / 2;
}

void LaserDetectorCalibrationDialog::emitBlobClosingSizeChanged() const
{
    int size = _blob_closing_size_sb->value();
    Q_ASSERT(size >=0);
    if(size > 0)
        size = 2 * size + 1;

    emit blobClosingSizeChanged(size);
}

void LaserDetectorCalibrationDialog::emitBlobPerimeterRangeChanged() const
{
    int min = _blob_perimeter_min_sb->value();
    _blob_perimeter_max_sb->setMinimum(min + 1);
    int max = _blob_perimeter_max_sb->value();

    Q_ASSERT(min > 0 && min < max);
    emit blobPerimeterRangeChanged(min, max);
}

void LaserDetectorCalibrationDialog::emitBlobCrownMarginsChanged() const
{
    int min = _blob_crown_margin_inf_sb->value();
    _blob_crown_margin_sup_sb->setMinimum(min + 1);
    int max = _blob_crown_margin_sup_sb->value();

    Q_ASSERT(min >= 0 && min < max);
    emit blobCrownMarginsChanged(min, max);
}

void LaserDetectorCalibrationDialog::writeSettings() const
{
    QSettings settings;
    settings.beginGroup("LaserDetectorCalibrationDialog");

    settings.setValue("dialog_size", size());

    settings.setValue("highest_brightness_min", _highest_brightness_min_sb->value());
    settings.setValue("relative_brightness_min", _relative_brightness_min_sb->value());
    settings.setValue("blob_closing_size", _blob_closing_size_sb->value());
    settings.setValue("nb_blobs_max", _nb_blobs_max_sb->value());
    settings.setValue("blob_perimeter_min", _blob_perimeter_min_sb->value());
    settings.setValue("blob_perimeter_max", _blob_perimeter_max_sb->value());
    settings.setValue("blob_crown_margin_inf", _blob_crown_margin_inf_sb->value());
    settings.setValue("blob_crown_margin_sup", _blob_crown_margin_sup_sb->value());
    settings.setValue("hue_mean", _hue_mean_sb->value());
    settings.setValue("hue_span", _hue_span_sb->value());
    settings.setValue("blob_crown_valid_pixels_part_min", _blob_crown_valid_pixels_part_min_sb->value());

    settings.endGroup();
}

} // namespace laser_painter