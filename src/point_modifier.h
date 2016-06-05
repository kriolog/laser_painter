#ifndef POINT_MODIFIER
#define POINT_MODIFIER

#include <QObject>
#include <QRect>

class QPoint;

namespace laser_painter {

/// Transorm points from the coordinate system of the scaled region of interest.
class PointModifier : public QObject
{
    Q_OBJECT

public:
    explicit PointModifier(QObject* parent = 0);

public slots:
    void run(const QPointF& point, bool found) const;
    /// Set (non-scaled) region of interest
    void setROI(const QRect& roi);
    void setUnscale(qreal unscale);

signals:
    /// Transformed point available
    void pointAvailable(const QPointF& point, bool found) const;

private:
    QRect _roi;
    qreal _unscale;
};

} // namespace laser_painter

#endif // POINT_MODIFIER