#include "image_modifier.h"

#include <QImage>

namespace laser_painter {

ImageModifier::ImageModifier(QObject* parent)
    : QObject(parent),
    _roi(),
    _scale(1.)
{}

void ImageModifier::run(const QImage& image) const
{
    if(image.isNull() || !QRect(QPoint(), image.size()).contains(_roi))
        return;

    QImage result = (_roi.isEmpty() || image.size() == _roi.size()) ? image : image.copy(_roi);
    if(_scale != 1.)
        result = result.scaled(result.width() * _scale, result.height() * _scale);

    if(!result.isNull())
        // Small images can become null after scale.
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