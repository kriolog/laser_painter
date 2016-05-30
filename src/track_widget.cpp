#include "track_widget.h"

#include <QPainter>
#include <QPointF>
#include <QSize>

namespace laser_painter {

TrackWidget::TrackWidget
(
    uint max_delay,
    int max_track_size,
    const QSize& canvas_size,
    QWidget* parent,
    Qt::WindowFlags f
) :
    QWidget(parent, f),
    _track(),
    _max_track_size(max_track_size),
    _delay(0),
    _max_delay(max_delay),
    _canvas_size(canvas_size)
{
    Q_ASSERT(_max_track_size > 0);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, Qt::black);
    setPalette(palette);
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
    _track.clear();
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
}

} // namespace laser_painter