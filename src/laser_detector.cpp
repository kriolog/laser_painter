#include "laser_detector.h"

#include <QImage>
#include <QPointF>
#include <QSize>

namespace laser_painter {

LaserDetector::LaserDetector(QObject* parent) :
    QObject(parent)
{}

void LaserDetector::run(const QImage& image)
{
    // TMP stub
    emit laserPosition(QPointF(0,0));
}

} // namespace laser_painter