#include "tracker_settings.h"

#include <QSettings>
#include <QPushButton>
#include <QSpinBox>
#include <QColorDialog>
#include <QVBoxLayout>
#include <QLabel>

#include "track_widget.h"

namespace laser_painter {

TrackerSettings::TrackerSettings(TrackWidget* track_widget, QWidget* parent):
    QGroupBox(parent)
{
    Q_ASSERT(track_widget);

    setTitle(tr("Tracker Settings"));

    QSettings settings;

    _max_track_size_sb = new QSpinBox();
    _max_track_size_sb->setRange(2, 99999);
    connect(_max_track_size_sb, SIGNAL(valueChanged(int)), track_widget, SLOT(setTrackMaxSize(int)));
    _max_track_size_sb->setValue(settings.value("TrackerSettings/max_track_size", 10000).toInt());
    QLabel* max_track_size_lb = new QLabel(tr("Track size:"));
    max_track_size_lb->setToolTip("Maximum number of track points");
    max_track_size_lb->setBuddy(_max_track_size_sb);

    _max_delay_sb = new QDoubleSpinBox();
    _max_delay_sb->setRange(0.1, 9.9);
    _max_delay_sb->setDecimals(1);
    _max_delay_sb->setSingleStep(.1);
    _max_delay_sb->setSuffix(tr(" s"));
    connect(_max_delay_sb, SIGNAL(valueChanged(double)), track_widget, SLOT(setTrackMaxDelay(double)));
    _max_delay_sb->setValue(settings.value("TrackerSettings/max_delay", 1.).toDouble());
    QLabel* max_delay_lb = new QLabel(tr("Delay:"));
    max_delay_lb->setToolTip("A new track begins after the delay has elapsed");
    max_delay_lb->setBuddy(_max_delay_sb);


    _track_color_bn = new QPushButton();
    _track_color_bn->setMaximumHeight(24);
    _track_color_bn->setMaximumWidth(32);
    _track_color_bn->setFocusPolicy(Qt::NoFocus);
    _track_color = settings.value("TrackerSettings/track_color", QColor(Qt::magenta)).value<QColor>();
    setColor(_track_color_bn, _track_color);
    connect(_track_color_bn, &QPushButton::clicked, this, &TrackerSettings::selectTrackColor);
    connect(this, &TrackerSettings::trackColorChanged, track_widget, &TrackWidget::setTrackColor);
    emit trackColorChanged(_track_color);
    QLabel* track_color_lb = new QLabel(tr("Track color:"));
    track_color_lb->setBuddy(_track_color_bn);

    _track_width_sb = new QSpinBox();
    _track_width_sb->setRange(1, 9);
    connect(_track_width_sb, SIGNAL(valueChanged(int)), track_widget, SLOT(setTrackWidth(int)));
    _track_width_sb->setValue(settings.value("TrackerSettings/track_width", 1).toInt());
    QLabel* track_width_lb = new QLabel(tr("Track width:"));
    track_width_lb->setToolTip("Track halfwidth");
    track_width_lb->setBuddy(_track_width_sb);

    _canvas_color_bn = new QPushButton();
    _canvas_color_bn->setMaximumHeight(24);
    _canvas_color_bn->setMaximumWidth(32);
    _canvas_color_bn->setFocusPolicy(Qt::NoFocus);
    _canvas_color = settings.value("TrackerSettings/canvas_color", QColor(Qt::black)).value<QColor>();
    setColor(_canvas_color_bn, _canvas_color);
    connect(_canvas_color_bn, &QPushButton::clicked, this, &TrackerSettings::selectCanvasColor);
    connect(this, &TrackerSettings::canvasColorChanged, track_widget, &TrackWidget::setCanvasColor);
    emit canvasColorChanged(_canvas_color);
    QLabel* canvas_color_lb = new QLabel(tr("Canvas color:"));
    canvas_color_lb->setBuddy(_canvas_color_bn);


    QHBoxLayout* max_delay_lo = new QHBoxLayout();
    max_delay_lo->addStretch();
    max_delay_lo->addWidget(max_delay_lb);
    max_delay_lo->addWidget(_max_delay_sb);

    QHBoxLayout* max_track_size_lo = new QHBoxLayout();
    max_track_size_lo->addStretch();
    max_track_size_lo->addWidget(max_track_size_lb);
    max_track_size_lo->addWidget(_max_track_size_sb);

    QHBoxLayout* track_color_lo = new QHBoxLayout();
    track_color_lo->addStretch();
    track_color_lo->addWidget(track_color_lb);
    track_color_lo->addWidget(_track_color_bn);

    QHBoxLayout* track_width_lo = new QHBoxLayout();
    track_width_lo->addStretch();
    track_width_lo->addWidget(track_width_lb);
    track_width_lo->addWidget(_track_width_sb);

    QHBoxLayout* canvas_color_lo = new QHBoxLayout();
    canvas_color_lo->addStretch();
    canvas_color_lo->addWidget(canvas_color_lb);
    canvas_color_lo->addWidget(_canvas_color_bn);

    QVBoxLayout* main_lo = new QVBoxLayout();
    main_lo->addLayout(max_delay_lo);
    main_lo->addLayout(max_track_size_lo);
    main_lo->addLayout(track_width_lo);
    main_lo->addLayout(track_color_lo);
    main_lo->addLayout(canvas_color_lo);
    setLayout(main_lo);
}

void TrackerSettings::selectTrackColor()
{
    selectColor(_track_color_bn, _track_color);
}

void TrackerSettings::selectCanvasColor()
{
    selectColor(_canvas_color_bn, _canvas_color);
}

void TrackerSettings::selectColor(QPushButton* color_bn, QColor& bn_color)
{
    Q_ASSERT(color_bn);

    QColor color = QColorDialog::getColor(color);

    if(!color.isValid())
        return;

    bn_color = color;
    setColor(color_bn, bn_color);
    color_bn->update();
    if(color_bn == _track_color_bn)
        emit trackColorChanged(bn_color);
    else if(color_bn == _canvas_color_bn)
        emit canvasColorChanged(bn_color);
    else
        Q_ASSERT(false);
}

void TrackerSettings::setColor(QPushButton* color_bn, const QColor& color)
{
    color_bn->setStyleSheet(QString("background-color: %1; border-style: solid; border-width: 1px; border-radius: 2px; border-color: %2;").arg(color.name()).arg(color.darker().name()));
}

void TrackerSettings::writeSettings() const
{
    QSettings settings;

    settings.beginGroup("TrackerSettings");

    settings.setValue("track_color", _track_color);
    settings.setValue("track_width", _track_width_sb->value());
    settings.setValue("canvas_color", _canvas_color);

    settings.setValue("max_delay", _max_delay_sb->value());
    settings.setValue("max_track_size", _max_track_size_sb->value());

    settings.endGroup();
}

} // namespace laser_painter