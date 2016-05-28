#ifndef TRACK_WIDGET
#define TRACK_WIDGET

#include <QWidget>
#include <QPolygonF>
#include <QSize>

class QPaintEvent;
class QPointF;

namespace laser_painter {

class TrackWidget: public QWidget
{
    Q_OBJECT

public:
    explicit TrackWidget
    (
        const QSize& canvas_size,
        uint max_delay,
        int track_max_size,
        QWidget* parent = 0,
        Qt::WindowFlags f = 0
    );

public slots:
    /// Add a new tip position @param pos to the track.
    /// If the current position was not @param found and delay is exceeded
    /// a new track is started.
    void addTip(const QPointF& pos, bool found);

    /// Set canvas size to @param canvas_size and start a new track.
    void setCanvasSize(const QSize& canvas_size);

protected:
    void paintEvent(QPaintEvent* event);

private:
    /// End the current track and start a new empty one.
    void startNewTrack();
    /// Add a new tip to the current track.
    void addTip(const QPointF& pos);

private:
    QPolygonF _track;
    int _max_track_size;
    // delay counter.
    uint _delay;
    // maximum delay.
    uint _max_delay;
    QSize _canvas_size;
};

} // namespace laser_painter

#endif // TRACK_WIDGET