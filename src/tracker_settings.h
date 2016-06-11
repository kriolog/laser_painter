#ifndef TRACKER_SETTINGS
#define TRACKER_SETTINGS

#include <QGroupBox>
#include <QColor>

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
    void selectColor(QPushButton* color_bn, QColor& bn_color);
    void setColor(QPushButton* color_bn, const QColor& color);

    void selectTrackColor();
    void selectCanvasColor();

signals:
    void trackColorChanged(const QColor& color) const;
    void canvasColorChanged(const QColor& color) const;

private:
    QSpinBox* _max_track_size_sb;
    QDoubleSpinBox* _max_delay_sb;
    QSpinBox* _track_width_sb;

    QPushButton* _track_color_bn;
    QPushButton* _canvas_color_bn;
    // TODO: No idea how to easily get a background color from a button style sheet.
    QColor _track_color;
    QColor _canvas_color;
};

} // namespace laser_painter

#endif // TRACKER_SETTINGS