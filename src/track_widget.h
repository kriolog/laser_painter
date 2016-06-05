#ifndef TRACK_WIDGET
#define TRACK_WIDGET

#include <QWidget>
#include <QPolygonF>
#include <QSize>
#include <QRect>

class QPaintEvent;
class QPointF;

namespace laser_painter {

class TrackWidget: public QWidget
{
    Q_OBJECT

public:
    /// @param max_delay maximal time between two successive addings of points
    /// When elapsed, a new track starts
    /// @param max_track_size maximal size of recorded track. If exceeded, the
    /// first added track point is removed each time.
    /// @param canvas_size size of canvas
    explicit TrackWidget
    (
        uint max_delay = 0,
        int max_track_size = 2,
        const QSize& canvas_size = QSize(),
        QWidget* parent = 0,
        Qt::WindowFlags flags = 0
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