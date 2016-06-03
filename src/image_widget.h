#ifndef IMAGE_WIDGET
#define IMAGE_WIDGET

#include <QWidget>
#include <QImage>

class QPaintEvent;

namespace laser_painter {

class ImageWidget: public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

public slots:
    /// Repaint with a new frame @param image.
    void setImage(const QImage& image);

protected:
    void paintEvent(QPaintEvent* event);

protected:
    // Current frame to paint.
    QImage _image;
    QPoint _image_origin;
};

} // namespace laser_painter

#endif // IMAGE_WIDGET