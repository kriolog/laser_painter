#include "point_modifier.h"

#include <QPoint>

namespace laser_painter {

PointModifier::PointModifier(QObject* parent)
    : QObject(parent),
    _roi(),
    _unscale(1.)
{}

void PointModifier::run(const QPointF& point, bool found) const
{
    Q_ASSERT(_unscale > 0);

    if(!found)
        emit pointAvailable(point, found);

    QPointF result = point;
    if(_unscale != 1.)
        result /= _unscale;
    result += _roi.topLeft();

    emit pointAvailable(result, found);
}

void PointModifier::setROI(const QRect& roi)
{
    _roi = roi;
}

void PointModifier::setUnscale(qreal unscale)
{
    _unscale = unscale;
}

} // namespace laser_painter