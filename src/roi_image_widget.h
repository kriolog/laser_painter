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
    /// Region of interest of the image with size @param image_rect is changed
    /// to @param roi.
    void roiChanged(const QRect& roi, const QSize& image_rect) const;

protected:
    // Selection handling
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void keyPressEvent(QKeyEvent *event);

private:
    void selectEntireImage(bool update_roi = true);
    // Update the input image size and scale roi to fit the new size.
    void updateInputGeometry(const QSize& input_image_size);
    void updateSelectionFromROI();
    void updateROIFromSelection();

private:
    QSize _input_image_size;
    // Region of interest (in a coordinate system of the input image)
    QRect _roi;
    // Current frame to paint.
    QRubberBand* _selection;
    // For mouse events only.
    QPoint _selection_origin;
};

} // namespace laser_painter

#endif // ROI_IMAGE_WIDGET