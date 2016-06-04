#include "roi_image_widget.h"

#include <QRubberBand>
#include <QMouseEvent>

namespace laser_painter {

ROIImageWidget::ROIImageWidget(QWidget* parent, Qt::WindowFlags f)
    : ImageWidget(parent, f),
    _selection_origin(),
    _selection(new QRubberBand(QRubberBand::Rectangle, this))
{
    selectEntireImage();
    _selection->show();
}

void ROIImageWidget::setImage(const QImage& image)
{
    bool update_input_geometry = _input_image_size != image.size();

    QPoint old_image_origin = _image_origin;
    QSize old_image_size = _image.size();

    if(update_input_geometry)
        updateInputGeometry(image.size());

    ImageWidget::setImage(image);

    if(update_input_geometry || _image.size() != old_image_size || _image_origin != old_image_origin)
        // Widget image geometry was changed, update selection.
        updateSelectionFromROI();
}

void ROIImageWidget::mousePressEvent(QMouseEvent *event)
{
    _selection_origin = event->pos();
    _selection->setGeometry(QRect(_selection_origin, QSize()));
}

void ROIImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    _selection->setGeometry(
        QRect(_selection_origin, event->pos())
        .intersected(QRect(_image_origin, _image.size()))
        .normalized()
    );
}

void ROIImageWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(_image.isNull())
        return;

    if(_selection->geometry().isEmpty())
        selectEntireImage();

    updateROIFromSelection();
}

void ROIImageWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
        selectEntireImage();
}

void ROIImageWidget::selectEntireImage(bool update_roi)
{
    _selection->setGeometry(QRect(_image_origin, _image.size()));
    if(update_roi)
        updateROIFromSelection();
}

void ROIImageWidget::updateInputGeometry(const QSize& input_image_size)
{
    if(_image.isNull() || _roi.size() == _input_image_size)
        // Set roi to 0 if the new image is invlaid
        // OR entire input image remains roi.
        _roi = QRect(QPoint(), input_image_size);
    else {
        qreal scale_x = static_cast<qreal>(input_image_size.width()) / _image.width();
        qreal scale_y = static_cast<qreal>(input_image_size.height()) / _image.height();
        _roi = QRect(
            QPoint(_roi.left() * scale_x, _roi.top() * scale_y),
            QSize(_roi.width() * scale_x, _roi.height() * scale_y)
        );
    }
    _input_image_size = input_image_size;
    emit roiChanged(_roi);
}

void ROIImageWidget::updateSelectionFromROI()
{
    if(_input_image_size.isEmpty() || _roi.size() == _input_image_size) {
        selectEntireImage(false);
        return;
    }

    // scale_x = scale_y because _image keeps the input image aspect ratio.
    qreal scale = static_cast<qreal>(_image.width()) / _input_image_size.width();
    QPoint selection_origin = _image_origin + _roi.topLeft() * scale;
    QSize selection_size(
        _roi.width() * scale,
        _roi.height() * scale
    );
    _selection->setGeometry(QRect(selection_origin, selection_size)
        .intersected(QRect(_image_origin, _image.size())));
}

void ROIImageWidget::updateROIFromSelection()
{
    if(_image.isNull() || _selection->size() == _image.size())
        _roi = QRect(QPoint(), _input_image_size);
    else {
        qreal unscale = static_cast<qreal>(_input_image_size.width()) / _image.width();
        // In the coordinate system of the input image
        QPoint roi_origin = (_selection->geometry().topLeft() - _image_origin) * unscale;
        QSize roi_size(
            _selection->width() * unscale,
            _selection->height() * unscale
        );
        _roi = QRect(roi_origin, roi_size)
            .intersected(QRect(QPoint(), _input_image_size));
    }
    emit roiChanged(_roi);

}


} // namespace laser_painter