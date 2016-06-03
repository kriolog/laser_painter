#include "roi_image_widget.h"

#include <QRubberBand>
#include <QMouseEvent>

namespace laser_painter {

ROIImageWidget::ROIImageWidget(QWidget* parent, Qt::WindowFlags f)
    : ImageWidget(parent, f),
    _rubber_band_origin(),
    _rubber_band(new QRubberBand(QRubberBand::Rectangle, this))
{
    selectEntireImage();
    _rubber_band->show();
}

void ROIImageWidget::setImage(const QImage& image)
{
    _input_image_size = image.size();

    QPoint old_image_origin = _image_origin;
    QSize old_image_size = _image.size();

    ImageWidget::setImage(image);

    if(old_image_origin != _image_origin || old_image_size != _image.size()) {
        // Image geometry was changed or image widget was resized.
        if(_rubber_band->size() == old_image_size) {
            // entire old image
            selectEntireImage();
            return;
        }

        if(old_image_size.isNull())
            return;

        // If the image aspect ratio is changed, width have priority.
        qreal scale_x = static_cast<qreal>(_image.width()) / old_image_size.width();
        qreal scale_y = static_cast<qreal>(_image.height()) / old_image_size.height();
        QPoint old_delta = _rubber_band_origin - old_image_origin;
        _rubber_band_origin = _image_origin + QPoint(old_delta.x() * scale_x, old_delta.y() * scale_y);
        _rubber_band->setGeometry(QRect(
            _rubber_band_origin,
            QSize(_rubber_band->width() * scale_x, _rubber_band->height() * scale_y)
        ));


        // TODO: copypaste;
        qreal unscale = static_cast<qreal>(_input_image_size.width()) / _image.width();
        // In the coordinate system of the input image
        QPoint rb_unscaled_origin = (_rubber_band_origin - _image_origin) * unscale;
        QSize rb_unscaled_size = QSize(
            _rubber_band->width() * unscale,
            _rubber_band->height() * unscale
        );
        QRect rb_unscaled_geometry = QRect(rb_unscaled_origin, rb_unscaled_size)
            .intersected(QRect(QPoint(), _input_image_size));
        emit roiChanged(rb_unscaled_geometry);
    }
}

void ROIImageWidget::mousePressEvent(QMouseEvent *event)
{
    _rubber_band_origin = event->pos();
    _rubber_band->setGeometry(QRect(_rubber_band_origin, QSize()));
}

void ROIImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    _rubber_band->setGeometry(
        QRect(_rubber_band_origin, event->pos())
        .intersected(QRect(_image_origin, _image.size()))
        .normalized()
    );
}

void ROIImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(_image.isNull())
        return;

    if(_rubber_band->geometry().isNull())
        selectEntireImage();

    qreal unscale = static_cast<qreal>(_input_image_size.width()) / _image.width();
    // In the coordinate system of the input image
    QPoint rb_unscaled_origin = (_rubber_band_origin - _image_origin) * unscale;
    QSize rb_unscaled_size = QSize(
        _rubber_band->width() * unscale,
        _rubber_band->height() * unscale
    );
    QRect rb_unscaled_geometry = QRect(rb_unscaled_origin, rb_unscaled_size)
        .intersected(QRect(QPoint(), _input_image_size));
    emit roiChanged(rb_unscaled_geometry);
}

void ROIImageWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
        selectEntireImage();
}

void ROIImageWidget::selectEntireImage()
{
    _rubber_band->setGeometry(QRect(_image_origin, _image.size()));
    emit roiChanged(QRect(QPoint(), _input_image_size));
}

} // namespace laser_painter