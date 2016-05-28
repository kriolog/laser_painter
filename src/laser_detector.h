#ifndef LASER_DETECTOR_H
#define LASER_DETECTOR_H

#include <QObject>

class QImage;
class QPointF;
class QSize;

namespace laser_painter {

/// Detect a laser dot position (or its absence) in the input image.
class LaserDetector : public QObject
{
    Q_OBJECT

public:
    explicit LaserDetector(QObject* parent = 0);

public slots:
    /// Run the detection for the input image @param image.
    /// @retval laserPosition signal
    void run(const QImage& image);

signals:
    /// Emit a laser dot position @param pos in the input image of
    /// size @param canvas_size and @param found = true if one and only one
    /// laser dot candidate has been found, otherwise return @param found = false.
    void laserPosition(const QPointF& pos, bool found = true);
};

} // namespace laser_painter

#endif // LASER_DETECTOR_H