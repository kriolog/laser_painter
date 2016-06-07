#ifndef TRACK_WIDGET
#define TRACK_WIDGET

#include <QWidget>
#include <QPolygonF>
#include <QSize>
#include <QRect>

class QPaintEvent;
class QPointF;
class QTimer;
class QColor;

namespace laser_painter {

class TrackWidget: public QWidget
{
    Q_OBJECT

public:
    /// @param max_delay maximal time (in seconds >= 0) between two successive
    /// addings of points
    /// When elapsed, a new track starts
    /// @param max_track_size maximal size of recorded track. If exceeded, the
    /// first added track point is removed each time.
    /// @param canvas_size size of canvas
    explicit TrackWidget
    (
        double max_delay = 0,
        int max_track_size = 2,
        const QSize& canvas_size = QSize(),
        QWidget* parent = 0,
        Qt::WindowFlags flags = 0
    );

public slots:
    /// Add a new tip position @param pos to the track.
    /// If the current position was not @param found a new track is started.
    void addTip(const QPointF& pos, bool found);

    /// Set canvas size to @param canvas_size and start a new track.
    void setCanvasSize(const QSize& canvas_size);

    /// @see TrackWidget()
    void setTrackMaxDelay(double delay);
    /// @see TrackWidget()
    /// @param size >= 2 (two points = 1 segmant)
    void setTrackMaxSize(int size);

    void setTrackColor(const QColor& color);
    void setTrackWidth(int halfwidth);
    void setCanvasColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event);

private:
    /// Add a new tip to the current track.
    void addTip(const QPointF& pos);

private slots:
    /// End the current track and start a new empty one.
    void startNewTrack();
    void updateOldTrackOpacity();
    void stopShowOldTrack();

private:
    // TODO: change to std::deque in the case of performance problems
    QPolygonF _track;
    int _max_track_size;
    // maximum delay.
    uint _max_delay;
    QSize _canvas_size;

    QColor _track_color;
    uint _track_width;
    QColor _canvas_color;

    QTimer* _fade_timer;
    QTimer* _max_delay_timer;
    QPolygonF _old_track;
    bool _show_old_track;
    uchar _old_track_opacity;
    static const int _fade_animation_time = 1024 * 1; // milliseconds
    static const int _fade_nb_steps = 32 * (_fade_animation_time / 1024);
    static const int _fade_timer_interval = _fade_animation_time / _fade_nb_steps;
    static const int _fade_opacity_step = 256 / _fade_nb_steps;
};

} // namespace laser_painter

#endif // TRACK_WIDGET