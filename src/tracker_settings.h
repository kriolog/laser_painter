#ifndef TRACKER_SETTINGS
#define TRACKER_SETTINGS

#include <QGroupBox>

class QPushButton;
class QSpinBox;
class QDoubleSpinBox;

namespace laser_painter {
    class TrackWidget;
}

namespace laser_painter {

class TrackerSettings: public QGroupBox
{
    Q_OBJECT

public:
    explicit TrackerSettings(
        TrackWidget* track_widget,
        QWidget* parent = 0
    );

    void writeSettings() const;

public slots:
    void selectColor(QPushButton* color_bn) const;
    void selectTrackColor() const;
    void selectCanvasColor() const;

signals:
    void trackColorChanged(const QColor& color) const;
    void canvasColorChanged(const QColor& color) const;

private:
    QSpinBox* _max_track_size_sb;
    QDoubleSpinBox* _max_delay_sb;
    QPushButton* _track_color_bn;
    QSpinBox* _track_width_sb;
    QPushButton* _canvas_color_bn;
};

} // namespace laser_painter

#endif // TRACKER_SETTINGS