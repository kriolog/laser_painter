#include "track_widget.h"

#include <QPainter>
#include <QPointF>
#include <QSize>

namespace laser_painter {

TrackWidget::TrackWidget
(
    const QSize& canvas_size,
    uint max_delay,
    int track_max_size,
    QWidget* parent,
    Qt::WindowFlags f
) :
    QWidget(parent, f),
    _track(),
    _max_track_size(track_max_size),
    _delay(0),
    _max_delay(max_delay),
    _canvas_size(canvas_size)
{
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

    QPainter painter(this);
    painter.setPen(QPen(Qt::magenta));
    painter.drawPolygon(_track);
}

} // namespace laser_painter