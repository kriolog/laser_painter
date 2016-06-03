#include "image_widget.h"

#include <QPainter>

namespace laser_painter {

ImageWidget::ImageWidget(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, Qt::black);
    setPalette(palette);
}

void ImageWidget::setImage(const QImage& image)
{
    // Prescale the image to fit the widget size before painting.
    // TODO: it's possible to save a QSharedPointer to the original image
    // and use painter->setTransform() in order to avoid this image copy.
    _image = image.scaled(size(), Qt::KeepAspectRatio);
    _image_origin.setX((width() - _image.width()) / 2);
    _image_origin.setY((height() - _image.height()) / 2);
    repaint();
}

void ImageWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if(_image.isNull())
        return;

    QPainter painter(this);
    painter.drawImage(_image_origin, _image);
}

} // namespace laser_painter