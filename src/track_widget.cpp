#include "track_widget.h"

#include <QPainter>
#include <QPointF>
#include <QSize>
#include <QTimer>

namespace laser_painter {

TrackWidget::TrackWidget
(
    uint max_delay,
    int max_track_size,
    const QSize& canvas_size,
    QWidget* parent,
    Qt::WindowFlags flags
)
    : QWidget(parent, flags),
    _track(),
    _max_track_size(max_track_size),
    _delay(0),
    _max_delay(max_delay),
    _canvas_size(canvas_size),
    _old_track(),
    _show_old_track(false),
    _old_track_opacity(255)
{
    Q_ASSERT(_max_track_size > 0);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, Qt::black);
    setPalette(palette);

    _fade_timer = new QTimer(this);
    _fade_timer->setInterval(_fade_timer_interval);
    connect(_fade_timer, &QTimer::timeout, this, &TrackWidget::updateOldTrackOpacity);
}

void TrackWidget::addTip(const QPointF& pos, bool found)
{
    if(!found) {
        if(++_delay > _max_delay) {
            if(!_track.isEmpty()) {
                startNewTrack();
            }
            _delay = 0;
        }
        return;
    }

    addTip(pos);
}

void TrackWidget::startNewTrack()
{
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
        _track.removeFirst();
    repaint();
}

void TrackWidget::setCanvasSize(const QSize& canvas_size)
{
    _canvas_size = canvas_size;
    startNewTrack();
}

void TrackWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if(_canvas_size.isEmpty())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::magenta);
    pen.setWidth(3);
    painter.setPen(pen);

    qreal scale_x = static_cast<qreal>(width()) / _canvas_size.width();
    qreal scale_y = static_cast<qreal>(height()) / _canvas_size.height();
    qreal scale = scale_x < scale_y ? scale_x : scale_y;
    painter.translate(
        (width() - scale * _canvas_size.width()) / 2.,
        (height() - scale * _canvas_size.height()) / 2.
    );
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