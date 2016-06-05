#ifndef CAMERA_SETTINGS
#define CAMERA_SETTINGS

#include <QWidget>

namespace laser_painter {
    class VideoFrameGrabber;
}

class QComboBox;
class QCamera;
class QCameraImageCapture;

namespace laser_painter {

class CameraSettings: public QWidget
{
    Q_OBJECT

public:
    explicit CameraSettings(
        VideoFrameGrabber* video_frame_grabber,
        QWidget* parent = 0, Qt::WindowFlags f = 0
    );

    void writeSettings() const;

signals:
    void cameraChanged(QCamera* camera);

private slots:
    void changeCamera();
    void updateAvailableCameras();

private:
    QComboBox* _camera_cb;
    QCamera* _camera;
    QCameraImageCapture* _camera_image_capture;

};

} // namespace laser_painter

#endif // CAMERA_SETTINGS