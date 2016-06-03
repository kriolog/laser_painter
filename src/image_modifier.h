#ifndef IMAGE_MODIFIER
#define IMAGE_MODIFIER

#include <QObject>
#include <QRect>

class QImage;

namespace laser_painter {

/// Scale and crop (by region of interest) of the input image.
class ImageModifier : public QObject
{
    Q_OBJECT

public:
    explicit ImageModifier(QObject* parent = 0);

public slots:
    void run(const QImage& image) const;
    /// Set region of interest (in a coordinate system of the input image).
    void setROI(const QRect& roi);
    void setScale(qreal scale);

signals:
    /// Modified image available
    void imageAvailable(const QImage& image) const;

private:
    QRect _roi;
    qreal _scale;
};

} // namespace laser_painter

#endif // IMAGE_MODIFIER