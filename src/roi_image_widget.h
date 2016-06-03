#ifndef ROI_IMAGE_WIDGET
#define ROI_IMAGE_WIDGET

#include "image_widget.h"

class QRubberBand;

namespace laser_painter {

/// Image widget with selection of region of interest
class ROIImageWidget: public ImageWidget
{
    Q_OBJECT

public:
    explicit ROIImageWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

public slots:
    void setImage(const QImage& image);

signals:
    /// Region of interest is changed to @param roi. @param image_size is
    /// the size of the input image.
    void roiChanged(const QRect& roi) const;

protected:
    // Rubber band handling
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *event);

private:
    void selectEntireImage();

private:
    // Current frame to paint.
    QRubberBand* _rubber_band;
    QPoint _rubber_band_origin;
    QSize _input_image_size;
};

} // namespace laser_painter

#endif // ROI_IMAGE_WIDGET