#include "track_widget.h"

#include <QPainter>
#include <QPointF>
#include <QSize>
#include <QTimer>
#include <QColor>

namespace laser_painter {

TrackWidget::TrackWidget
(
    double max_delay,
    int max_track_size,
    const QSize& canvas_size,
    QWidget* parent,
    Qt::WindowFlags flags
)
    : QWidget(parent, flags),
    _track(),
    _max_track_size(max_track_size),
    _max_delay(max_delay * 1000),
    _canvas_size(canvas_size),
    _track_color(Qt::magenta),
    _track_width(3),
    _canvas_color(Qt::black),
    _old_track(),
    _show_old_track(false),
    _old_track_opacity(255)
{
    Q_ASSERT(max_track_size > 0);
    Q_ASSERT(max_delay >= 0);

    _fade_timer = new QTimer(this);
    _fade_timer->setInterval(_fade_timer_interval);
    connect(_fade_timer, &QTimer::timeout, this, &TrackWidget::updateOldTrackOpacity);

    _max_delay_timer = new QTimer(this);
    connect(_max_delay_timer, &QTimer::timeout, this, &TrackWidget::startNewTrack);
    _max_delay_timer->setInterval(_max_delay);
    _max_delay_timer->setSingleShot(true);
    _max_delay_timer->start();
}

void TrackWidget::addTip(const QPointF& pos, bool found)
{
    if(found) {
        addTip(pos);
        // Restart delay timer
        _max_delay_timer->start();
    }
}

void TrackWidget::startNewTrack()
{
    _max_delay_timer->start();

    if(_track.isEmpty())
        return;

    if(_show_old_track)
        stopShowOldTrack();

    _old_track.clear();
    _track.swap(_old_track);
    _show_old_track = true;
    _fade_timer->start();
    repaint();
}

void TrackWidget::addTip(const QPointF& pos)
{
    _track.append(pos);
    if(_track.size() > _max_track_size)
        _track.remove(0, _track.size() - _max_track_size);
    repaint();
}

void TrackWidget::setCanvasSize(const QSize& canvas_size)
{
    _canvas_size = canvas_size;
    _track.clear(); // prevent painting irrelevant old track after rescaling
    startNewTrack();
}

void TrackWidget::setTrackMaxDelay(double delay)
{
    Q_ASSERT(delay >= 0);
    _max_delay = delay * 1000;
    _max_delay_timer->setInterval(_max_delay);
    _max_delay_timer->start();
}

void TrackWidget::setTrackMaxSize(int size)
{
    Q_ASSERT(size > 1);
    _max_track_size = size;
}

void TrackWidget::setTrackColor(const QColor& color)
{
    Q_ASSERT(color.isValid());
    _track_color = color;
}

void TrackWidget::setTrackWidth(int halfwidth)
{
    Q_ASSERT(halfwidth > 0);
    _track_width = 2 * halfwidth - 1;
}

void TrackWidget::setCanvasColor(const QColor& color)
{
    Q_ASSERT(color.isValid());
    _canvas_color = color;
}

void TrackWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if(_canvas_size.isEmpty())
        return;

    qreal scale_x = static_cast<qreal>(width()) / _canvas_size.width();
    qreal scale_y = static_cast<qreal>(height()) / _canvas_size.height();
    qreal scale = scale_x < scale_y ? scale_x : scale_y;

    QSize scaled_canvas_size = QSize(
        scale * _canvas_size.width(),
        scale * _canvas_size.height()
    );
    QPoint scaled_canvas_origin = QPoint(
        (width() - scaled_canvas_size.width()) / 2.,
        (height() - scaled_canvas_size.height()) / 2.
    );

    // Draw canvas
    QPainter painter(this);
    painter.setBrush(QBrush(_canvas_color));
    painter.drawRect(QRect(scaled_canvas_origin, scaled_canvas_size));

    // Draw track
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(_track_color);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(_track_width);
    painter.setPen(pen);
    painter.translate(scaled_canvas_origin);
    painter.scale(scale, scale);
    painter.drawPolyline(_track);

    if(_show_old_track) {
        QColor pen_color = pen.color();
        pen_color.setAlpha(_old_track_opacity);
        pen.setColor(pen_color);
        painter.setPen(pen);
        painter.drawPolyline(_old_track);
    }
}

void TrackWidget::updateOldTrackOpacity()
{
    if(_old_track_opacity <= _fade_opacity_step)
        stopShowOldTrack();
    _old_track_opacity -= _fade_opacity_step;
    repaint();
}

void TrackWidget::stopShowOldTrack()
{
    _old_track_opacity = 255;
    _fade_timer->stop();
    _show_old_track = false;
}

} // namespace laser_painter