#include "image_modifier.h"

#include <QImage>
#include <QDebug>

namespace laser_painter {

ImageModifier::ImageModifier(QObject* parent)
    : QObject(parent),
    _roi(),
    _scale(1.)
{}

void ImageModifier::run(const QImage& image) const
{
    Q_ASSERT(QRect(QPoint(), image.size()).contains(_roi));

    QImage result = _roi.isNull() ? image : image.copy(_roi);
    if(_scale != 1.)
        result = result.scaled(result.width() * _scale, result.height() * _scale);

    emit imageAvailable(result);
}

void ImageModifier::setROI(const QRect& roi)
{
    _roi = roi;
}

void ImageModifier::setScale(qreal scale)
{
    _scale = scale;
}

} // namespace laser_painter