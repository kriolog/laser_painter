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
    _current_image = image.scaled(size(), Qt::KeepAspectRatio);
    repaint();
}

void ImageWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    if(_current_image.isNull()) {
        return;
    }

    QPainter painter(this);
    QPoint center;
    center.setX((width() - _current_image.width()) / 2);
    center.setY((height() - _current_image.height()) / 2);
    painter.drawImage(center, _current_image);
}

} // namespace laser_painter